////////////////////////////////////////////////////////////////////////////////
/// @brief V8 engine configuration
///
/// @file
///
/// DISCLAIMER
///
/// Copyright 2004-2012 triagens GmbH, Cologne, Germany
///
/// Licensed under the Apache License, Version 2.0 (the "License");
/// you may not use this file except in compliance with the License.
/// You may obtain a copy of the License at
///
///     http://www.apache.org/licenses/LICENSE-2.0
///
/// Unless required by applicable law or agreed to in writing, software
/// distributed under the License is distributed on an "AS IS" BASIS,
/// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
/// See the License for the specific language governing permissions and
/// limitations under the License.
///
/// Copyright holder is triAGENS GmbH, Cologne, Germany
///
/// @author Dr. Frank Celler
/// @author Copyright 2011-2012, triAGENS GmbH, Cologne, Germany
////////////////////////////////////////////////////////////////////////////////

#include "ApplicationV8.h"

#include "Basics/ConditionLocker.h"
#include "Basics/ReadLocker.h"
#include "Basics/WriteLocker.h"
#include "Logger/Logger.h"
#include "V8/v8-conv.h"
#include "V8/v8-shell.h"
#include "V8/v8-utils.h"
#include "V8Server/v8-actions.h"
#include "V8Server/v8-query.h"
#include "V8Server/v8-vocbase.h"

using namespace triagens;
using namespace triagens::basics;
using namespace triagens::arango;
using namespace std;

// -----------------------------------------------------------------------------
// --SECTION--                                                  class V8GcThread
// -----------------------------------------------------------------------------

////////////////////////////////////////////////////////////////////////////////
/// @addtogroup ArangoDB
/// @{
////////////////////////////////////////////////////////////////////////////////

namespace {

////////////////////////////////////////////////////////////////////////////////
/// @brief garbage collector
////////////////////////////////////////////////////////////////////////////////

  class V8GcThread : public Thread {
    public:
      V8GcThread (ApplicationV8* applicationV8) 
        : Thread("v8-gc"),
          _applicationV8(applicationV8),
          _lock(),
          _lastGcStamp(TRI_microtime()) {
      }

    public:

////////////////////////////////////////////////////////////////////////////////
/// @brief collect garbage in an endless loop (main functon of GC thread)
////////////////////////////////////////////////////////////////////////////////

      void run () {
        _applicationV8->collectGarbage();
      }

////////////////////////////////////////////////////////////////////////////////
/// @brief get the timestamp of the last GC
////////////////////////////////////////////////////////////////////////////////

      double getLastGcStamp () {
        READ_LOCKER(_lock);
        return _lastGcStamp;
      }

////////////////////////////////////////////////////////////////////////////////
/// @brief set the global GC timestamp 
////////////////////////////////////////////////////////////////////////////////

      void updateGcStamp (double value) {
        WRITE_LOCKER(_lock);
        _lastGcStamp = value;
      }

    private:
      ApplicationV8* _applicationV8;
      ReadWriteLock _lock;
      double _lastGcStamp;
  };
}

////////////////////////////////////////////////////////////////////////////////
/// @}
////////////////////////////////////////////////////////////////////////////////

// -----------------------------------------------------------------------------
// --SECTION--                                                      public types
// -----------------------------------------------------------------------------

////////////////////////////////////////////////////////////////////////////////
/// @addtogroup ArangoDB
/// @{
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
/// @brief adds a global method
////////////////////////////////////////////////////////////////////////////////

void ApplicationV8::V8Context::addGlobalContextMethod (string const& method) {
  _globalMethods.push_back(method);
}

////////////////////////////////////////////////////////////////////////////////
/// @brief executes all global methods
////////////////////////////////////////////////////////////////////////////////

void ApplicationV8::V8Context::handleGlobalContextMethods () {
  v8::HandleScope scope;

  for (vector<string>::iterator i = _globalMethods.begin();  i != _globalMethods.end();  ++i) {
    string const& func = *i;

    LOGGER_DEBUG("executing global context methods '" << func << "' for context " << _id);

    TRI_ExecuteJavaScriptString(_context,
                                v8::String::New(func.c_str()),
                                v8::String::New("global context method"),
                                false);
  }

  _globalMethods.clear();
}

////////////////////////////////////////////////////////////////////////////////
/// @}
////////////////////////////////////////////////////////////////////////////////

// -----------------------------------------------------------------------------
// --SECTION--                                               class ApplicationV8
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
// --SECTION--                                      constructors and destructors
// -----------------------------------------------------------------------------

////////////////////////////////////////////////////////////////////////////////
/// @addtogroup ArangoDB
/// @{
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
/// @brief constructor
////////////////////////////////////////////////////////////////////////////////

ApplicationV8::ApplicationV8 (string const& binaryPath)
  : ApplicationFeature("V8"),
    _startupPath(),
    _startupModules(),
    _startupNodeModules(),
    _actionPath(),
    _useActions(true),
    _performUpgrade(false),
    _skipUpgrade(false),
    _gcInterval(1000),
    _gcFrequency(10.0),
    _v8Options(""),
    _startupLoader(),
    _actionLoader(),
    _vocbase(0),
    _nrInstances(0),
    _contexts(0),
    _contextCondition(),
    _freeContexts(),
    _dirtyContexts(),
    _busyContexts(),
    _stopping(0) {
}

////////////////////////////////////////////////////////////////////////////////
/// @brief destructor
////////////////////////////////////////////////////////////////////////////////

ApplicationV8::~ApplicationV8 () {
}

////////////////////////////////////////////////////////////////////////////////
/// @}
////////////////////////////////////////////////////////////////////////////////

// -----------------------------------------------------------------------------
// --SECTION--                                                    public methods
// -----------------------------------------------------------------------------

////////////////////////////////////////////////////////////////////////////////
/// @addtogroup ArangoDB
/// @{
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
/// @brief sets the concurrency
////////////////////////////////////////////////////////////////////////////////

void ApplicationV8::setConcurrency (size_t n) {
  _nrInstances = n;
}

////////////////////////////////////////////////////////////////////////////////
/// @brief sets the database
////////////////////////////////////////////////////////////////////////////////

void ApplicationV8::setVocbase (TRI_vocbase_t* vocbase) {
  _vocbase = vocbase;
}

////////////////////////////////////////////////////////////////////////////////
/// @brief perform a database upgrade
////////////////////////////////////////////////////////////////////////////////

void ApplicationV8::performUpgrade () {
  _performUpgrade = true;
}

////////////////////////////////////////////////////////////////////////////////
/// @brief skip a database upgrade
////////////////////////////////////////////////////////////////////////////////

void ApplicationV8::skipUpgrade () {
  _skipUpgrade = true;
}

////////////////////////////////////////////////////////////////////////////////
/// @brief enters a context
////////////////////////////////////////////////////////////////////////////////

ApplicationV8::V8Context* ApplicationV8::enterContext () {
  CONDITION_LOCKER(guard, _contextCondition);

  while (_freeContexts.empty() && ! _stopping) {
    LOGGER_DEBUG("waiting for unused V8 context");
    guard.wait();
  }

  // in case we are in the shutdown phase, do not enter a context!
  // the context might have been deleted by the shutdown
  if (_stopping) {
    return 0;
  }

  LOGGER_TRACE("found unused V8 context");

  V8Context* context = _freeContexts.back();

  assert(context != 0);
  assert(context->_isolate != 0);

  _freeContexts.pop_back();
  _busyContexts.insert(context);

  context->_locker = new v8::Locker(context->_isolate);
  context->_isolate->Enter();
  context->_context->Enter();

  context->handleGlobalContextMethods();

  return context;
}

////////////////////////////////////////////////////////////////////////////////
/// @brief exists an context
////////////////////////////////////////////////////////////////////////////////

void ApplicationV8::exitContext (V8Context* context) {
  V8GcThread* gc = dynamic_cast<V8GcThread*>(_gcThread);
  assert(gc != 0);

  double lastGc = gc->getLastGcStamp();

  CONDITION_LOCKER(guard, _contextCondition);

  context->handleGlobalContextMethods();

  context->_context->Exit();
  context->_isolate->Exit();
  delete context->_locker;
  
  ++context->_dirt;

  if (context->_lastGcStamp + _gcFrequency < lastGc) {
    LOGGER_TRACE("V8 context has reached GC timeout threshold and will be scheduled for GC");
    _dirtyContexts.push_back(context);
    _busyContexts.erase(context);
  }
  else if (context->_dirt >= _gcInterval) {
    LOGGER_TRACE("V8 context has reached maximum number of requests and will be scheduled for GC");
    _dirtyContexts.push_back(context);
    _busyContexts.erase(context);
  }
  else {
    _freeContexts.push_back(context);
    _busyContexts.erase(context);
  }
  
  guard.broadcast();

  LOGGER_TRACE("returned dirty V8 context");
}

////////////////////////////////////////////////////////////////////////////////
/// @brief adds a global context functions to be executed asap
////////////////////////////////////////////////////////////////////////////////

void ApplicationV8::addGlobalContextMethod (string const& method) {
  CONDITION_LOCKER(guard, _contextCondition);
  
  for (vector<V8Context*>::iterator i = _freeContexts.begin();  i != _freeContexts.end();  ++i) {
    V8Context* context = *i;

    context->addGlobalContextMethod(method);
  }

  for (vector<V8Context*>::iterator i = _dirtyContexts.begin();  i != _dirtyContexts.end();  ++i) {
    V8Context* context = *i;

    context->addGlobalContextMethod(method);
  }

  for (set<V8Context*>::iterator i = _busyContexts.begin();  i != _busyContexts.end();  ++i) {
    V8Context* context = *i;

    context->addGlobalContextMethod(method);
  }
}

////////////////////////////////////////////////////////////////////////////////
/// @brief determine which of the free contexts should be picked for the GC
////////////////////////////////////////////////////////////////////////////////

ApplicationV8::V8Context* ApplicationV8::pickContextForGc () {
  size_t n = _freeContexts.size();

  if (n == 0) {
    // this is easy...
    return 0;
  }

  V8GcThread* gc = dynamic_cast<V8GcThread*>(_gcThread);
  V8Context* context = 0;

  // we got more than 1 context to clean up, pick the one with the "oldest" GC stamp
  size_t pickedContextNr = 0; // index of context with lowest GC stamp
  
  for (size_t i = 0; i < n; ++i) {
    // compare last GC stamp
    if (_freeContexts[i]->_lastGcStamp <= _freeContexts[pickedContextNr]->_lastGcStamp) {
      pickedContextNr = i;
    }
  }
  // we now have the context to clean up in pickedContextNr
    
  // this is the context to clean up
  context = _freeContexts[pickedContextNr];
  assert(context != 0);

  // now compare its last GC timestamp with the last global GC stamp
  if (context->_lastGcStamp + _gcFrequency >= gc->getLastGcStamp()) {
    // no need yet to clean up the context
    return 0;
  }

  // we'll pop the context from the vector. the context might be at any position in the vector
  // so we need to move the other elements around
  if (n > 1) {
    for (size_t i = pickedContextNr; i < n - 1; ++i) {
      _freeContexts[i] = _freeContexts[i + 1];
    }
  }
  _freeContexts.pop_back();

  return context;
}

////////////////////////////////////////////////////////////////////////////////
/// @brief runs the garbage collection
////////////////////////////////////////////////////////////////////////////////

void ApplicationV8::collectGarbage () {
  V8GcThread* gc = dynamic_cast<V8GcThread*>(_gcThread);
  assert(gc != 0);

  // this flag will be set to true if we timed out waiting for a GC signal
  // if set to true, the next cycle will use a reduced wait time so the GC
  // can be performed more early for all dirty contexts. The flag is set
  // to false again once all contexts have been cleaned up and there is nothing
  // more to do
  bool useReducedWait = false;

  // the time we'll wait for a signal
  const uint64_t regularWaitTime = (uint64_t) (_gcFrequency * 1000.0 * 1000.0);

  // the time we'll wait for a signal when the previous wait timed out
  const uint64_t reducedWaitTime = (uint64_t) (_gcFrequency * 1000.0 * 100.0);

  while (_stopping == 0) {
    V8Context* context = 0;

    {
      bool gotSignal = false;
      CONDITION_LOCKER(guard, _contextCondition);

      if (_dirtyContexts.empty()) {
        uint64_t waitTime = useReducedWait ? reducedWaitTime : regularWaitTime;

        // we'll wait for a signal or a timeout
        gotSignal = guard.wait(waitTime);

        // use a reduced wait time in the next round because we seem to be idle
        // the reduced wait time will allow use to perfom GC for more contexts
        useReducedWait = ! gotSignal;
      }

      if (! _dirtyContexts.empty()) {
        context = _dirtyContexts.back();
        _dirtyContexts.pop_back();
        useReducedWait = false;
      }
      else if (! gotSignal && ! _freeContexts.empty()) {
        // we timed out waiting for a signal, so we have idle time that we can
        // spend on running the GC pro-actively
        // We'll pick one of the free contexts and clean it up
        context = pickContextForGc();

        // there is no context to clean up, probably they all have been cleaned up
        // already. increase the wait time so we don't cycle too much in the GC loop
        // and waste CPU unnecessary
        useReducedWait =  (context != 0);
      }
    }
   
    // update last gc time   
    double lastGc = TRI_microtime();
    gc->updateGcStamp(lastGc);

    if (context != 0) {
      LOGGER_TRACE("collecting V8 garbage");

      context->_locker = new v8::Locker(context->_isolate);
      context->_isolate->Enter();
      context->_context->Enter();

      v8::V8::LowMemoryNotification();
      while (! v8::V8::IdleNotification()) {
      }

      context->_context->Exit();
      context->_isolate->Exit();
      delete context->_locker;

      context->_dirt = 0;
      context->_lastGcStamp = lastGc;

      {
        CONDITION_LOCKER(guard, _contextCondition);

        _freeContexts.push_back(context);
        guard.broadcast();
      }
    }
  }
}

////////////////////////////////////////////////////////////////////////////////
/// @brief disables actions
////////////////////////////////////////////////////////////////////////////////

void ApplicationV8::disableActions () {
  _useActions = false;
}

////////////////////////////////////////////////////////////////////////////////
/// @}
////////////////////////////////////////////////////////////////////////////////

// -----------------------------------------------------------------------------
// --SECTION--                                        ApplicationFeature methods
// -----------------------------------------------------------------------------

////////////////////////////////////////////////////////////////////////////////
/// @addtogroup ApplicationServer
/// @{
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
/// {@inheritDoc}
////////////////////////////////////////////////////////////////////////////////

void ApplicationV8::setupOptions (map<string, basics::ProgramOptionsDescription>& options) {
  options["JAVASCRIPT Options:help-admin"]
    ("javascript.gc-interval", &_gcInterval, "JavaScript request-based garbage collection interval (each x requests)")
    ("javascript.gc-frequency", &_gcFrequency, "JavaScript time-based garbage collection frequency (each x seconds)")
    ("javascript.action-directory", &_actionPath, "path to the JavaScript action directory")
    ("javascript.modules-path", &_startupModules, "one or more directories separated by (semi-) colons")
    ("javascript.package-path", &_startupNodeModules, "one or more directories separated by (semi-) colons")
    ("javascript.startup-directory", &_startupPath, "path to the directory containing alternate JavaScript startup scripts")
    ("javascript.v8-options", &_v8Options, "options to pass to v8")
  ;
}

////////////////////////////////////////////////////////////////////////////////
/// {@inheritDoc}
////////////////////////////////////////////////////////////////////////////////

bool ApplicationV8::prepare () {
  LOGGER_DEBUG("V8 version: " << v8::V8::GetVersion()); 

  // use a minimum of 1 second for GC
  if (_gcFrequency < 1) {
    _gcFrequency = 1;
  }

  // check the startup modules
  if (_startupModules.empty()) {
    LOGGER_FATAL_AND_EXIT("no 'javascript.modules-path' has been supplied, giving up");
  }
  else {
    LOGGER_INFO("using JavaScript modules path '" << _startupModules << "'");
  }

  if (! _startupNodeModules.empty()) {
    LOGGER_INFO("using Node modules path '" << _startupNodeModules << "'");
  }

  // set up the startup loader
  if (_startupPath.empty()) {
    LOGGER_FATAL_AND_EXIT("no 'javascript.startup-directory' has been supplied, giving up");
  }
  else {
    LOGGER_INFO("using JavaScript startup files at '" << _startupPath << "'");
    _startupLoader.setDirectory(_startupPath);
  }

  // set up action loader
  if (_useActions) {
    if (_actionPath.empty()) {
      LOGGER_FATAL_AND_EXIT("no 'javascript.action-directory' has been supplied, giving up");
    }
    else {
      LOGGER_INFO("using JavaScript action files at '" << _actionPath << "'");

      _actionLoader.setDirectory(_actionPath);
    }
  }

  // add v8 options
  if (_v8Options.size() > 0) {
    LOGGER_INFO("using V8 options '" << _v8Options << "'");
    v8::V8::SetFlagsFromString(_v8Options.c_str(), _v8Options.size());
  }

  // setup instances
  _contexts = new V8Context*[_nrInstances];

  for (size_t i = 0;  i < _nrInstances;  ++i) {
    bool ok = prepareV8Instance(i);

    if (! ok) {
      return false;
    }
  }

  return true;
}

////////////////////////////////////////////////////////////////////////////////
/// {@inheritDoc}
////////////////////////////////////////////////////////////////////////////////

bool ApplicationV8::start () {
  _gcThread = new V8GcThread(this);
  _gcThread->start();

  return true;
}

////////////////////////////////////////////////////////////////////////////////
/// {@inheritDoc}
////////////////////////////////////////////////////////////////////////////////

void ApplicationV8::close () {
  _stopping = 1;
  _contextCondition.broadcast();
}

////////////////////////////////////////////////////////////////////////////////
/// {@inheritDoc}
////////////////////////////////////////////////////////////////////////////////

void ApplicationV8::stop () {

  // stop GC
  _gcThread->shutdown();

  // shutdown all action threads
  for (size_t i = 0;  i < _nrInstances;  ++i) {
    shutdownV8Instance(i);
  }

  delete[] _contexts;

  // delete GC thread after all action threads have been stopped
  delete _gcThread;
}

////////////////////////////////////////////////////////////////////////////////
/// @}
////////////////////////////////////////////////////////////////////////////////

// -----------------------------------------------------------------------------
// --SECTION--                                                   private methods
// -----------------------------------------------------------------------------

////////////////////////////////////////////////////////////////////////////////
/// @addtogroup ArangoDB
/// @{
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
/// @brief prepares a V8 instance
////////////////////////////////////////////////////////////////////////////////

bool ApplicationV8::prepareV8Instance (const size_t i) {
  vector<string> files;

  files.push_back("common/bootstrap/modules.js");
  files.push_back("common/bootstrap/module-internal.js");
  files.push_back("common/bootstrap/module-fs.js");
  files.push_back("common/bootstrap/module-console.js"); // needs internal
  files.push_back("common/bootstrap/errors.js");
  files.push_back("common/bootstrap/monkeypatches.js");

  files.push_back("server/bootstrap/module-internal.js");
  files.push_back("server/server.js"); // needs internal

  LOGGER_TRACE("initialising V8 context #" << i);

  V8Context* context = _contexts[i] = new V8Context();
  if (context == 0) {
    LOGGER_FATAL_AND_EXIT("cannot initialize V8 engine");
  }

  // enter a new isolate
  context->_id = i;
  context->_isolate = v8::Isolate::New();
  context->_locker = new v8::Locker(context->_isolate);
  context->_isolate->Enter();

  // create the context
  context->_context = v8::Context::New();

  if (context->_context.IsEmpty()) {
    LOGGER_FATAL_AND_EXIT("cannot initialize V8 engine");
  }

  context->_context->Enter();

  TRI_InitV8VocBridge(context->_context, _vocbase, i);
  TRI_InitV8Queries(context->_context);


  if (_useActions) {
    TRI_InitV8Actions(context->_context, this);
  }

  TRI_InitV8Conversions(context->_context);
  TRI_InitV8Utils(context->_context, _startupModules, _startupNodeModules);
  TRI_InitV8Shell(context->_context);

  // set global flag before loading system files
  if (i == 0 && ! _skipUpgrade) {
    v8::HandleScope scope;
    context->_context->Global()->Set(v8::String::New("UPGRADE"), _performUpgrade ? v8::True() : v8::False());
  }

  // load all init files
  for (size_t j = 0;  j < files.size();  ++j) {
    bool ok = _startupLoader.loadScript(context->_context, files[j]);

    if (! ok) {
      LOGGER_FATAL_AND_EXIT("cannot load JavaScript utilities from file '" << files[j] << "'");
    }
  }

  // run upgrade script
  if (i == 0 && ! _skipUpgrade) {
    LOGGER_DEBUG("running database version check");

    // special check script to be run just once in first thread (not in all)
    v8::HandleScope scope;
    v8::Handle<v8::Value> result = _startupLoader.executeGlobalScript(context->_context, "server/version-check.js");
  
    if (! TRI_ObjectToBoolean(result)) {
      if (_performUpgrade) {
        LOGGER_FATAL_AND_EXIT("Database upgrade failed. Please inspect the logs from the upgrade procedure");
      }
      else {
        LOGGER_FATAL_AND_EXIT("Database version check failed. Please start the server with the --upgrade option");
      }
    }
    
    LOGGER_DEBUG("database version check passed");
  }

  if (_performUpgrade) {
    // issue #391: when invoked with --upgrade, the server will not always shut down
    LOGGER_INFO("database version check passed");
    context->_context->Exit();
    context->_isolate->Exit();
    delete context->_locker;
    
    TRI_EXIT_FUNCTION(EXIT_SUCCESS, NULL);
  }

  // load all actions
  if (_useActions) {
    bool ok = _actionLoader.executeAllScripts(context->_context);

    if (! ok) {
      LOGGER_FATAL_AND_EXIT("cannot load JavaScript actions from directory '" << _actionLoader.getDirectory() << "'");
    }

    {
      v8::HandleScope scope;
      TRI_ExecuteJavaScriptString(context->_context,
                                  v8::String::New("require(\"internal\").actionLoaded()"),
                                  v8::String::New("action loaded"),
                                  false);
    }
  }

  // and return from the context
  context->_context->Exit();
  context->_isolate->Exit();
  delete context->_locker;

  context->_lastGcStamp = TRI_microtime();
  
  LOGGER_TRACE("initialised V8 context #" << i);

  _freeContexts.push_back(context);

  return true;
}

////////////////////////////////////////////////////////////////////////////////
/// @brief shut downs a V8 instances
////////////////////////////////////////////////////////////////////////////////

void ApplicationV8::shutdownV8Instance (size_t i) {
  LOGGER_TRACE("shutting down V8 context #" << i);

  V8Context* context = _contexts[i];

  context->_locker = new v8::Locker(context->_isolate);
  context->_isolate->Enter();
  context->_context->Enter();

  v8::V8::LowMemoryNotification();
  while (! v8::V8::IdleNotification()) {
  }
 
  TRI_v8_global_t* v8g = (TRI_v8_global_t*) context->_isolate->GetData();

  if (v8g) {
    delete v8g;
  }

  context->_context->Exit();
  context->_context.Dispose();

  context->_isolate->Exit();
  delete context->_locker;

  context->_isolate->Dispose();

  delete context;

  LOGGER_TRACE("closed V8 context #" << i);
}

////////////////////////////////////////////////////////////////////////////////
/// @}
////////////////////////////////////////////////////////////////////////////////

// Local Variables:
// mode: outline-minor
// outline-regexp: "^\\(/// @brief\\|/// {@inheritDoc}\\|/// @addtogroup\\|// --SECTION--\\|/// @\\}\\)"
// End:

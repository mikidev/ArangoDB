////////////////////////////////////////////////////////////////////////////////
/// @brief scheduler thread
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
/// @author Martin Schoenert
/// @author Copyright 2009-2012, triAGENS GmbH, Cologne, Germany
////////////////////////////////////////////////////////////////////////////////

#include "SchedulerThread.h"

#include "Logger/Logger.h"
#include "Scheduler/Scheduler.h"
#include "Scheduler/Task.h"

using namespace triagens::basics;
using namespace triagens::rest;

#ifdef TRI_USE_SPIN_LOCK_SCHEDULER_THREAD
#define SCHEDULER_INIT TRI_InitSpin
#define SCHEDULER_DESTROY TRI_DestroySpin
#define SCHEDULER_LOCK TRI_LockSpin
#define SCHEDULER_UNLOCK TRI_UnlockSpin
#else
#define SCHEDULER_INIT TRI_InitMutex
#define SCHEDULER_DESTROY TRI_DestroyMutex
#define SCHEDULER_LOCK TRI_LockMutex
#define SCHEDULER_UNLOCK TRI_UnlockMutex
#endif

// -----------------------------------------------------------------------------
// --SECTION--                                      constructors and destructors
// -----------------------------------------------------------------------------

////////////////////////////////////////////////////////////////////////////////
/// @addtogroup Scheduler
/// @{
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
/// @brief constructor
////////////////////////////////////////////////////////////////////////////////

SchedulerThread::SchedulerThread (Scheduler* scheduler, EventLoop loop, bool defaultLoop)
  : Thread("scheduler"),
    scheduler(scheduler),
    defaultLoop(defaultLoop),
    loop(loop),
    stopping(0),
    stopped(0),
    hasWork(0) {

  // init lock
  SCHEDULER_INIT(&queueLock);
  
  // allow cancelation
  allowAsynchronousCancelation();
}

////////////////////////////////////////////////////////////////////////////////
/// @brief destructor
////////////////////////////////////////////////////////////////////////////////

SchedulerThread::~SchedulerThread () {
  SCHEDULER_DESTROY(&queueLock);
}

////////////////////////////////////////////////////////////////////////////////
/// @}
////////////////////////////////////////////////////////////////////////////////

// -----------------------------------------------------------------------------
// --SECTION--                                                    public methods
// -----------------------------------------------------------------------------

////////////////////////////////////////////////////////////////////////////////
/// @addtogroup Scheduler
/// @{
////////////////////////////////////////////////////////////////////////////////

void SchedulerThread::beginShutdown () {
  LOGGER_TRACE << "beginning shutdown sequence of scheduler thread (" << threadId() << ")";
      
  stopping = 1;
  scheduler->wakeupLoop(loop);
}

////////////////////////////////////////////////////////////////////////////////
/// @brief registers a task
////////////////////////////////////////////////////////////////////////////////

void SchedulerThread::registerTask (Scheduler* scheduler, Task* task) {

  // thread has already been stopped
  if (stopped) {
  }

  // same thread, in this case it does not matter if we are inside the loop
  else if (threadId() == currentThreadId()) {
    setupTask(task, scheduler, loop);
    scheduler->wakeupLoop(loop);
  }

  // different thread, be careful - we have to stop the event loop
  else {

    // put the register request unto the queue
    SCHEDULER_LOCK(&queueLock);

    Work w(SETUP, scheduler, task);
    queue.push_back(w);
    hasWork = 1;

    scheduler->wakeupLoop(loop);

    SCHEDULER_UNLOCK(&queueLock);
  }
}

////////////////////////////////////////////////////////////////////////////////
/// @brief unregisters a task
////////////////////////////////////////////////////////////////////////////////

void SchedulerThread::unregisterTask (Task* task) {
  deactivateTask(task);

  // thread has already been stopped
  if (stopped) {
  }

  // same thread, in this case it does not matter if we are inside the loop
  else if (threadId() == currentThreadId()) {
    cleanupTask(task);
    scheduler->wakeupLoop(loop);
  }

  // different thread, be careful - we have to stop the event loop
  else {

    // put the unregister request unto the queue
    SCHEDULER_LOCK(&queueLock);

    Work w(CLEANUP, 0, task);
    queue.push_back(w);
    hasWork = 1;

    scheduler->wakeupLoop(loop);

    SCHEDULER_UNLOCK(&queueLock);
  }
}

////////////////////////////////////////////////////////////////////////////////
/// @brief unregisters a task
////////////////////////////////////////////////////////////////////////////////

void SchedulerThread::destroyTask (Task* task) {
  deactivateTask(task);

  // thread has already been stopped
  if (stopped) {
    deleteTask(task);
  }

  // same thread, in this case it does not matter if we are inside the loop
  else if (threadId() == currentThreadId()) {
    cleanupTask(task);
    deleteTask(task);
    scheduler->wakeupLoop(loop);
  }

  // different thread, be careful - we have to stop the event loop
  else {
    
    // put the unregister request unto the queue
    SCHEDULER_LOCK(&queueLock);
    
    Work w(DESTROY, 0, task);
    queue.push_back(w);
    hasWork = 1;
    
    scheduler->wakeupLoop(loop);
    
    SCHEDULER_UNLOCK(&queueLock);
  }
}

////////////////////////////////////////////////////////////////////////////////
/// @}
////////////////////////////////////////////////////////////////////////////////

// -----------------------------------------------------------------------------
// --SECTION--                                                    Thread methods
// -----------------------------------------------------------------------------

////////////////////////////////////////////////////////////////////////////////
/// @addtogroup Scheduler
/// @{
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
/// {@inheritDoc}
////////////////////////////////////////////////////////////////////////////////

void SchedulerThread::run () {
  LOGGER_TRACE << "scheduler thread started (" << threadId() << ")";

  if (defaultLoop) {
    sigset_t all;
    sigemptyset(&all);
    
    pthread_sigmask(SIG_SETMASK, &all, 0);
  }

  while (stopping == 0) {
    try {
      scheduler->eventLoop(loop);
    }
    catch (...) {
#ifdef TRI_HAVE_POSIX_THREADS
      if (stopping != 0) {
        LOGGER_WARNING << "caught cancellation exception during work";
        throw;
      }
#endif

      LOGGER_WARNING << "caught exception from ev_loop";
    }

#if defined(DEBUG_SCHEDULER_THREAD)
    LOGGER_TRACE << "left scheduler loop " << threadId();
#endif

    if (hasWork != 0) {
      SCHEDULER_LOCK(&queueLock);
      
      while (! queue.empty()) {
        Work w = queue.front();
        queue.pop_front();
        
        SCHEDULER_UNLOCK(&queueLock);
        
        switch (w.work) {
          case CLEANUP:
            cleanupTask(w.task);
            break;
            
          case SETUP:
            setupTask(w.task, w.scheduler, loop);
            break;
            
          case DESTROY:
            cleanupTask(w.task);
            deleteTask(w.task);
            break;
        }

        SCHEDULER_LOCK(&queueLock);
      }
      
      hasWork = 0;
      
      SCHEDULER_UNLOCK(&queueLock);
    }
  }

  LOGGER_TRACE << "scheduler thread stopped (" << threadId() << ")";

  stopped = 1;

  SCHEDULER_LOCK(&queueLock);
      
  while (! queue.empty()) {
    Work w = queue.front();
    queue.pop_front();
        
    SCHEDULER_UNLOCK(&queueLock);
        
    switch (w.work) {
      case CLEANUP:
        break;
            
      case SETUP:
        break;
            
      case DESTROY:
        deleteTask(w.task);
        break;
    }

    SCHEDULER_LOCK(&queueLock);
  }
      
  SCHEDULER_UNLOCK(&queueLock);
}

////////////////////////////////////////////////////////////////////////////////
/// @}
////////////////////////////////////////////////////////////////////////////////

// -----------------------------------------------------------------------------
// --SECTION--                                                       END-OF-FILE
// -----------------------------------------------------------------------------

// Local Variables:
// mode: outline-minor
// outline-regexp: "^\\(/// @brief\\|/// {@inheritDoc}\\|/// @addtogroup\\|// --SECTION--\\|/// @\\}\\)"
// End:
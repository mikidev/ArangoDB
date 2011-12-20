////////////////////////////////////////////////////////////////////////////////
/// @brief job dispatcher
///
/// @file
///
/// DISCLAIMER
///
/// Copyright 2010-2011 triagens GmbH, Cologne, Germany
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
/// @author Copyright 2009-2011, triAGENS GmbH, Cologne, Germany
////////////////////////////////////////////////////////////////////////////////

#ifndef TRIAGENS_FYN_DISPATCHER_DISPATCHER_IMPL_H
#define TRIAGENS_FYN_DISPATCHER_DISPATCHER_IMPL_H 1

#include <BasicsC/Common.h>

#include <Rest/Dispatcher.h>
#include <Basics/Mutex.h>

namespace triagens {
  namespace rest {

    /////////////////////////////////////////////////////////////////////////////
    /// @brief job dispatcher
    /////////////////////////////////////////////////////////////////////////////

    class DispatcherImpl : public Dispatcher {
      public:

        /////////////////////////////////////////////////////////////////////////
        /// @brief queue thread creator
        /////////////////////////////////////////////////////////////////////////

        typedef DispatcherThread* (*newDispatcherThread_fptr)(DispatcherQueue*);

      public:

        /////////////////////////////////////////////////////////////////////////
        /// @brief default queue thread creator
        /////////////////////////////////////////////////////////////////////////

        static DispatcherThread* defaultDispatcherThread (DispatcherQueue*);

      public:

        /////////////////////////////////////////////////////////////////////////
        /// @brief constructs a new dispatcher
        /////////////////////////////////////////////////////////////////////////

        DispatcherImpl ();

        /////////////////////////////////////////////////////////////////////////
        /// @brief destructs the dispatcher
        /////////////////////////////////////////////////////////////////////////

        virtual ~DispatcherImpl ();

      public:

        /////////////////////////////////////////////////////////////////////////
        /// {@inheritDoc}
        /////////////////////////////////////////////////////////////////////////

        bool isRunning ();

        /////////////////////////////////////////////////////////////////////////
        /// {@inheritDoc}
        /////////////////////////////////////////////////////////////////////////

        void addQueue (string const& name, size_t nrThreads);

        /////////////////////////////////////////////////////////////////////////
        /// {@inheritDoc}
        /////////////////////////////////////////////////////////////////////////

        bool addJob (Job*);

        /////////////////////////////////////////////////////////////////////////
        /// {@inheritDoc}
        /////////////////////////////////////////////////////////////////////////

        bool start ();

        /////////////////////////////////////////////////////////////////////////
        /// {@inheritDoc}
        /////////////////////////////////////////////////////////////////////////

        void beginShutdown ();

        /////////////////////////////////////////////////////////////////////////
        /// {@inheritDoc}
        /////////////////////////////////////////////////////////////////////////

        void reportStatus ();

      public:

        /////////////////////////////////////////////////////////////////////////
        /// @brief adds a queue which given dispatcher thread type
        /////////////////////////////////////////////////////////////////////////

        void addQueue (string const& name, newDispatcherThread_fptr, size_t nrThreads);

      protected:

        /////////////////////////////////////////////////////////////////////////
        /// @brief looks up a queue
        /////////////////////////////////////////////////////////////////////////

        DispatcherQueue* lookupQueue (string const&);

      private:
        basics::Mutex accessDispatcher;
        volatile sig_atomic_t stopping;

        map<string, DispatcherQueue*> queues;
    };
  }
}

#endif

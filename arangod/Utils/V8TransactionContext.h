////////////////////////////////////////////////////////////////////////////////
/// @brief V8 transaction context
///
/// @file
///
/// DISCLAIMER
///
/// Copyright 2004-2013 triAGENS GmbH, Cologne, Germany
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
/// @author Jan Steemann
/// @author Copyright 2011-2013, triAGENS GmbH, Cologne, Germany
////////////////////////////////////////////////////////////////////////////////

#ifndef TRIAGENS_UTILS_V8TRANSACTION_CONTEXT_H
#define TRIAGENS_UTILS_V8TRANSACTION_CONTEXT_H 1

#include "V8/v8-globals.h"

namespace triagens {
  namespace arango {

    class V8TransactionContext {

// -----------------------------------------------------------------------------
// --SECTION--                                        class V8TransactionContext
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
// --SECTION--                                      constructors and destructors
// -----------------------------------------------------------------------------

////////////////////////////////////////////////////////////////////////////////
/// @addtogroup ArangoDB
/// @{
////////////////////////////////////////////////////////////////////////////////

      public:

////////////////////////////////////////////////////////////////////////////////
/// @brief create the context
////////////////////////////////////////////////////////////////////////////////

        V8TransactionContext () : _previous(0) {
          TRI_v8_global_t* v8g;

          v8g = (TRI_v8_global_t*) v8::Isolate::GetCurrent()->GetData();

          if (v8g->_currentTransaction != 0) {
            _previous = (TRI_transaction_t*) v8g->_currentTransaction;
          }
        }

////////////////////////////////////////////////////////////////////////////////
/// @brief destroy the context
////////////////////////////////////////////////////////////////////////////////

        ~V8TransactionContext () {
        }

////////////////////////////////////////////////////////////////////////////////
/// @}
////////////////////////////////////////////////////////////////////////////////

// -----------------------------------------------------------------------------
// --SECTION--                                               protected functions
// -----------------------------------------------------------------------------

////////////////////////////////////////////////////////////////////////////////
/// @addtogroup ArangoDB
/// @{
////////////////////////////////////////////////////////////////////////////////

      protected:

////////////////////////////////////////////////////////////////////////////////
/// @brief whether or not the transaction is embedded
////////////////////////////////////////////////////////////////////////////////

        inline bool isEmbedded () const {
          return _previous != 0;
        }

////////////////////////////////////////////////////////////////////////////////
/// @brief return the parent transaction if any
////////////////////////////////////////////////////////////////////////////////

        inline TRI_transaction_t* getParent () const {
          return _previous;
        }

////////////////////////////////////////////////////////////////////////////////
/// @brief register the transaction in the context
////////////////////////////////////////////////////////////////////////////////

        int registerTransaction (TRI_transaction_t* const trx) {
          TRI_v8_global_t* v8g;
          v8g = (TRI_v8_global_t*) v8::Isolate::GetCurrent()->GetData();
          v8g->_currentTransaction = trx;

          return TRI_ERROR_NO_ERROR;
        }

////////////////////////////////////////////////////////////////////////////////
/// @brief unregister the transaction from the context
////////////////////////////////////////////////////////////////////////////////

        int unregisterTransaction () {
          TRI_v8_global_t* v8g;

          v8g = (TRI_v8_global_t*) v8::Isolate::GetCurrent()->GetData();
          v8g->_currentTransaction = 0;

          return TRI_ERROR_NO_ERROR;
        }

////////////////////////////////////////////////////////////////////////////////
/// @}
////////////////////////////////////////////////////////////////////////////////

// -----------------------------------------------------------------------------
// --SECTION--                                                 private variables
// -----------------------------------------------------------------------------

////////////////////////////////////////////////////////////////////////////////
/// @addtogroup ArangoDB
/// @{
////////////////////////////////////////////////////////////////////////////////

      private:

////////////////////////////////////////////////////////////////////////////////
/// @brief previous transaction
////////////////////////////////////////////////////////////////////////////////

        TRI_transaction_t* _previous;

////////////////////////////////////////////////////////////////////////////////
/// @}
////////////////////////////////////////////////////////////////////////////////

    };

  }
}

#endif

// Local Variables:
// mode: outline-minor
// outline-regexp: "/// @brief\\|/// {@inheritDoc}\\|/// @addtogroup\\|/// @page\\|// --SECTION--\\|/// @\\}"
// End:

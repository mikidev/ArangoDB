////////////////////////////////////////////////////////////////////////////////
/// @brief wrapper for self-contained, single collection write transactions
///
/// @file
///
/// DISCLAIMER
///
/// Copyright 2004-2012 triAGENS GmbH, Cologne, Germany
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
/// @author Copyright 2011-2012, triAGENS GmbH, Cologne, Germany
////////////////////////////////////////////////////////////////////////////////

#ifndef TRIAGENS_UTILS_SINGLE_COLLECTION_WRITE_TRANSACTION_H
#define TRIAGENS_UTILS_SINGLE_COLLECTION_WRITE_TRANSACTION_H 1

#include "Utils/CollectionNameResolver.h"
#include "Utils/SingleCollectionTransaction.h"

#include "ShapedJson/shaped-json.h"
#include "VocBase/transaction.h"
#include "VocBase/vocbase.h"

namespace triagens {
  namespace arango {

    template<typename T, uint64_t N>
    class SingleCollectionWriteTransaction : public SingleCollectionTransaction<T> {

// -----------------------------------------------------------------------------
// --SECTION--                            class SingleCollectionWriteTransaction
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
/// @brief create the transaction, using a collection object
///
/// A single collection write transaction executes write operations on the
/// underlying collection. It can be limited to at most one write operation.
/// If so, whenever it tries to execute more than one write operation, an error 
/// can be raised automatically. Executing only one write operation is
/// sufficient for the basic CRUD operations and allows using the transaction
/// API even efficiently for single CRUD operations. This optimisation will
/// allow avoiding a lot of the general transaction overhead.
////////////////////////////////////////////////////////////////////////////////

        SingleCollectionWriteTransaction (TRI_vocbase_t* const vocbase,
                                          const triagens::arango::CollectionNameResolver& resolver,
                                          const TRI_transaction_cid_t cid) :
          SingleCollectionTransaction<T>(vocbase, resolver, cid, TRI_TRANSACTION_WRITE),
          _numWrites(0), 
          _synchronous(false) {

          if (N == 1) {
            this->addHint(TRI_TRANSACTION_HINT_SINGLE_OPERATION);
          }
        }

////////////////////////////////////////////////////////////////////////////////
/// @brief same as above, but create using collection name
////////////////////////////////////////////////////////////////////////////////

        SingleCollectionWriteTransaction (TRI_vocbase_t* const vocbase,
                                          const triagens::arango::CollectionNameResolver& resolver,
                                          const string& name) :
          SingleCollectionTransaction<T>(vocbase, resolver, resolver.getCollectionId(name), TRI_TRANSACTION_WRITE),
          _numWrites(0), 
          _synchronous(false) {

          if (N == 1) {
            this->addHint(TRI_TRANSACTION_HINT_SINGLE_OPERATION);
          }
        }
        
////////////////////////////////////////////////////////////////////////////////
/// @brief end the transaction
////////////////////////////////////////////////////////////////////////////////

        virtual ~SingleCollectionWriteTransaction () {
        }

////////////////////////////////////////////////////////////////////////////////
/// @}
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
/// @addtogroup ArangoDB
/// @{
////////////////////////////////////////////////////////////////////////////////

// -----------------------------------------------------------------------------
// --SECTION--                                                  public functions
// -----------------------------------------------------------------------------

      public:

////////////////////////////////////////////////////////////////////////////////
/// @brief return whether the last write in a transaction was synchronous
////////////////////////////////////////////////////////////////////////////////

        inline bool synchronous () const {
          return _synchronous;
        }

////////////////////////////////////////////////////////////////////////////////
/// @brief explicitly lock the underlying collection for write access
////////////////////////////////////////////////////////////////////////////////

        int lockWrite () {
          TRI_primary_collection_t* primary = this->primaryCollection();

          return this->lockExplicit(primary, TRI_TRANSACTION_WRITE);
        }

////////////////////////////////////////////////////////////////////////////////
/// @brief create a single document within a transaction, using json
////////////////////////////////////////////////////////////////////////////////

        int createDocument (TRI_doc_mptr_t* mptr,
                            TRI_json_t const* json, 
                            const bool forceSync,
                            const bool lock) {
          if (_numWrites++ > N) {
            return TRI_ERROR_TRANSACTION_INTERNAL;
          }

          TRI_primary_collection_t* primary = this->primaryCollection();
          _synchronous = forceSync || primary->base._info._waitForSync;

          return this->createCollectionDocument(primary, TRI_DOC_MARKER_KEY_DOCUMENT, mptr, json, 0, forceSync, lock);
        }

////////////////////////////////////////////////////////////////////////////////
/// @brief create a single edge within a transaction, using json
////////////////////////////////////////////////////////////////////////////////

        int createEdge (TRI_doc_mptr_t* mptr,
                        TRI_json_t const* json, 
                        bool forceSync, 
                        void const* data,
                        const bool lock) {
          if (_numWrites++ > N) {
            return TRI_ERROR_TRANSACTION_INTERNAL;
          }

          TRI_primary_collection_t* primary = this->primaryCollection();
          _synchronous = forceSync || primary->base._info._waitForSync;
          
          return this->createCollectionDocument(primary, TRI_DOC_MARKER_KEY_EDGE, mptr, json, data, forceSync, lock);
        }

////////////////////////////////////////////////////////////////////////////////
/// @brief create a single document within a transaction, using shaped json
////////////////////////////////////////////////////////////////////////////////

        int createDocument (TRI_voc_key_t key,
                            TRI_doc_mptr_t* mptr,
                            TRI_shaped_json_t const* shaped, 
                            bool forceSync,
                            const bool lock) {
          if (_numWrites++ > N) {
            return TRI_ERROR_TRANSACTION_INTERNAL;
          }

          TRI_primary_collection_t* primary = this->primaryCollection();
          _synchronous = forceSync || primary->base._info._waitForSync;
          
          return this->createCollectionShaped(primary, TRI_DOC_MARKER_KEY_DOCUMENT, key, mptr, shaped, 0, forceSync, lock);
        }

////////////////////////////////////////////////////////////////////////////////
/// @brief create a single edge within a transaction, using shaped json
////////////////////////////////////////////////////////////////////////////////

        int createEdge (TRI_voc_key_t key,
                        TRI_doc_mptr_t* mptr,
                        TRI_shaped_json_t const* shaped, 
                        bool forceSync, 
                        void const* data,
                        const bool lock) {
          if (_numWrites++ > N) {
            return TRI_ERROR_TRANSACTION_INTERNAL;
          }

          TRI_primary_collection_t* primary = this->primaryCollection();
          _synchronous = forceSync || primary->base._info._waitForSync;
          
          return this->createCollectionShaped(primary, TRI_DOC_MARKER_KEY_EDGE, key, mptr, shaped, data, forceSync, lock);
        }

////////////////////////////////////////////////////////////////////////////////
/// @brief update (replace!) a single document within a transaction, 
/// using json
////////////////////////////////////////////////////////////////////////////////

        int updateDocument (const string& key,
                            TRI_doc_mptr_t* mptr, 
                            TRI_json_t* const json, 
                            const TRI_doc_update_policy_e policy, 
                            bool forceSync, 
                            const TRI_voc_rid_t expectedRevision, 
                            TRI_voc_rid_t* actualRevision,
                            const bool lock) {
          if (_numWrites++ > N) {
            return TRI_ERROR_TRANSACTION_INTERNAL;
          }

          TRI_primary_collection_t* primary = this->primaryCollection();
          _synchronous = forceSync || primary->base._info._waitForSync;

          return this->updateCollectionDocument(primary, key, mptr, json, policy, expectedRevision, actualRevision, forceSync, lock);
        }

////////////////////////////////////////////////////////////////////////////////
/// @brief update (replace!) a single document within a transaction, 
/// using shaped json
////////////////////////////////////////////////////////////////////////////////

        int updateDocument (const string& key,
                            TRI_doc_mptr_t* mptr, 
                            TRI_shaped_json_t* const shaped, 
                            const TRI_doc_update_policy_e policy, 
                            bool forceSync, 
                            const TRI_voc_rid_t expectedRevision, 
                            TRI_voc_rid_t* actualRevision,
                            const bool lock) {
          if (_numWrites++ > N) {
            return TRI_ERROR_TRANSACTION_INTERNAL;
          }

          TRI_primary_collection_t* primary = this->primaryCollection();
          _synchronous = forceSync || primary->base._info._waitForSync;

          return this->updateCollectionShaped(primary, key, mptr, shaped, policy, expectedRevision, actualRevision, forceSync, lock);
        }

////////////////////////////////////////////////////////////////////////////////
/// @brief delete a single document within a transaction
////////////////////////////////////////////////////////////////////////////////

        int deleteDocument (const string& key, 
                            const TRI_doc_update_policy_e policy, 
                            bool forceSync, 
                            const TRI_voc_rid_t expectedRevision, 
                            TRI_voc_rid_t* actualRevision) {
          if (_numWrites++ > N) {
            return TRI_ERROR_TRANSACTION_INTERNAL;
          }

          TRI_primary_collection_t* primary = this->primaryCollection();
          _synchronous = forceSync || primary->base._info._waitForSync;

          return this->deleteCollectionDocument(primary, key, policy, expectedRevision, actualRevision, forceSync);
        }

////////////////////////////////////////////////////////////////////////////////
/// @brief truncate all documents within a transaction
////////////////////////////////////////////////////////////////////////////////

        int truncate (bool forceSync) {
          if (_numWrites++ > N) {
            return TRI_ERROR_TRANSACTION_INTERNAL;
          }

          TRI_primary_collection_t* primary = this->primaryCollection();
          _synchronous = forceSync || primary->base._info._waitForSync;

          return this->truncateCollection(primary, forceSync);
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
/// @brief number of writes the transaction has executed
/// the value should be 0 or 1, but not get any higher because values higher
/// than one indicate internal errors
////////////////////////////////////////////////////////////////////////////////

        uint64_t _numWrites;

////////////////////////////////////////////////////////////////////////////////
/// @brief whether of not the last write action was executed synchronously
////////////////////////////////////////////////////////////////////////////////

        bool _synchronous;

////////////////////////////////////////////////////////////////////////////////
/// @}
////////////////////////////////////////////////////////////////////////////////

    };

  }
}

#endif

// Local Variables:
// mode: outline-minor
// outline-regexp: "^\\(/// @brief\\|/// {@inheritDoc}\\|/// @addtogroup\\|/// @page\\|// --SECTION--\\|/// @\\}\\)"
// End:

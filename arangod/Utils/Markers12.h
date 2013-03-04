////////////////////////////////////////////////////////////////////////////////
/// @brief marker types from ArangoDB 1.2
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
/// @author Copyright 2013, triAGENS GmbH, Cologne, Germany
////////////////////////////////////////////////////////////////////////////////

#ifndef TRIAGENS_ARANGOD_UTILS_MARKERS12_H
#define TRIAGENS_ARANGOD_UTILS_MARKERS12_H 1

#include "VocBase/datafile.h"
#include "VocBase/vocbase.h"

// -----------------------------------------------------------------------------
// --SECTION--                                           Arango 1.2 marker types
// -----------------------------------------------------------------------------

namespace triagens {
  namespace arango {
    namespace markers12 {

// -----------------------------------------------------------------------------
// --SECTION--                                                      public types
// -----------------------------------------------------------------------------

////////////////////////////////////////////////////////////////////////////////
/// @addtogroup VocBase
/// @{
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
/// @brief transaction id type used in 1.2
////////////////////////////////////////////////////////////////////////////////

      typedef uint64_t voc_tid_t;

////////////////////////////////////////////////////////////////////////////////
/// @brief step type used in 1.2
////////////////////////////////////////////////////////////////////////////////

      typedef uint64_t voc_eid_t;

////////////////////////////////////////////////////////////////////////////////
/// @brief document marker used in 1.2
////////////////////////////////////////////////////////////////////////////////

      typedef struct {
        TRI_df_marker_t   base;
  
        TRI_voc_rid_t     _rid;          // 8 bytes, this is the tick for a create and update
        voc_eid_t         _sid;          // 8 bytes 

        TRI_shape_sid_t   _shape;        // 8 bytes 
 
        uint16_t          _offsetKey;    // 2 bytes
        uint16_t          _offsetJson;   // 2 bytes

#ifdef TRI_PADDING_32
        char              _padding_df_marker[4];    // 4 bytes
#endif
      }
      doc_document_marker_t;

////////////////////////////////////////////////////////////////////////////////
/// @brief edge marker used in 1.2
////////////////////////////////////////////////////////////////////////////////

      typedef struct {
        doc_document_marker_t  base;

        TRI_voc_cid_t          _toCid;
        TRI_voc_cid_t          _fromCid;   
  
        uint16_t               _offsetToKey;   // 2 bytes
        uint16_t               _offsetFromKey; // 2 bytes

#ifdef TRI_PADDING_32
        char                   _padding_df_marker[4];    // 4 bytes
#endif
      }
      doc_edge_marker_t;

////////////////////////////////////////////////////////////////////////////////
/// @brief deletion marker used in 1.2
////////////////////////////////////////////////////////////////////////////////

      typedef struct {
        TRI_df_marker_t   base;
  
        TRI_voc_rid_t     _rid;      // 8 bytes, this is the tick for an create and update
        voc_eid_t         _sid;      // 8 bytes

        uint16_t       _offsetKey;   // 2 bytes
      }
      doc_deletion_marker_t;

////////////////////////////////////////////////////////////////////////////////
/// @}
////////////////////////////////////////////////////////////////////////////////

// -----------------------------------------------------------------------------
// --SECTION--                                                  public functions
// -----------------------------------------------------------------------------

////////////////////////////////////////////////////////////////////////////////
/// @addtogroup VocBase
/// @{
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
/// @brief convert a 1.2 document marker
////////////////////////////////////////////////////////////////////////////////

      int64_t convertDocumentMarker (char* payload, 
                                     const off_t paddedSize, 
                                     int* err,
                                     TRI_datafile_t* df, 
                                     int fdout, 
                                     const TRI_server_id_t serverId) {
        *err = TRI_ERROR_NO_ERROR;

        doc_document_marker_t* oldMarker = (doc_document_marker_t*) payload;

        TRI_doc_document_key_marker_t newMarker;
        TRI_voc_size_t newMarkerSize = sizeof(TRI_doc_document_key_marker_t);

        char* body = ((char*) oldMarker) + oldMarker->_offsetJson;
        TRI_voc_size_t bodySize = oldMarker->base._size - sizeof(doc_document_marker_t); 
        TRI_voc_size_t bodySizePadded = paddedSize - sizeof(doc_document_marker_t); 
            
        char* keyBody;
        TRI_voc_size_t keyBodySize; 
        TRI_voc_size_t keySize;
        
        char* oldKey = (char*) oldMarker + oldMarker->_offsetKey;
        
        keySize = strlen(oldKey) + 1;
        keyBodySize = TRI_DF_ALIGN_BLOCK(keySize);
        keyBody = (char*) TRI_Allocate(TRI_CORE_MEM_ZONE, keyBodySize, true);
        TRI_CopyString(keyBody, oldKey, keySize);      

        TRI_sequence_value_t sequenceValue = TRI_SEQUENCE_VALUE(oldMarker->_rid);
        newMarker._tid = sequenceValue;
        TRI_UpdateGlobalIdSequence(sequenceValue);

        newMarker._shape      = oldMarker->_shape;
        newMarker._offsetKey  = newMarkerSize;
        newMarker._offsetJson = newMarkerSize + keyBodySize;
        
        LOG_DEBUG("found doc marker, type: %d, key: '%s', rid: %llu, size: %d, crc: %lu", 
                  (int) oldMarker->base._type, 
                  oldKey, 
                  (unsigned long long)  oldMarker->_rid,
                  (int) oldMarker->base._size,
                  (unsigned long) oldMarker->base._crc);
           
        TRI_InitMarkerDatafile(&newMarker.base, newMarkerSize + keyBodySize + bodySize, TRI_DOC_MARKER_KEY_DOCUMENT, serverId, sequenceValue);
        TRI_FillCrcKeyMarkerDatafile(df, &newMarker.base, newMarkerSize, keyBody, keyBodySize, body, bodySize);
        
        LOG_DEBUG("new doc marker, type: %d, size: %d, serverId: %llu, sequenceValue: %llu, key: '%s', keyBodySize: %d, bodySize: %d, bodySizePadded: %d, tid: %llu, crc: %lu", 
                  (int) newMarker.base._type, 
                  (int) newMarker.base._size,
                  (unsigned long long) serverId,
                  (unsigned long long) sequenceValue,
                  keyBody,
                  (int) keyBodySize,
                  (int) bodySize,
                  (int) bodySizePadded,
                  (unsigned long long) newMarker._tid,
                  (unsigned long) newMarker.base._crc);

        ssize_t writeResult;
        writeResult = TRI_WRITE(fdout, &newMarker, sizeof(newMarker));
        if (writeResult == 0) {
          *err = TRI_ERROR_INTERNAL;
        }
        writeResult = TRI_WRITE(fdout, keyBody, keyBodySize);
        if (writeResult == 0) {
          *err = TRI_ERROR_INTERNAL;
        }
        writeResult = TRI_WRITE(fdout, body, bodySizePadded);
        if (writeResult == 0) {
          *err = TRI_ERROR_INTERNAL;
        }

        TRI_Free(TRI_CORE_MEM_ZONE, keyBody);

        return sizeof(newMarker) + keyBodySize + bodySizePadded;
      }

////////////////////////////////////////////////////////////////////////////////
/// @brief convert a 1.2 edge marker
////////////////////////////////////////////////////////////////////////////////
      
      int64_t convertEdgeMarker (char* payload, 
                                 const off_t paddedSize, 
                                 int* err,
                                 TRI_datafile_t* df, 
                                 int fdout, 
                                 const TRI_server_id_t serverId) {
        *err = TRI_ERROR_NO_ERROR;

        doc_edge_marker_t* oldMarker = (doc_edge_marker_t*) payload;            

        TRI_doc_edge_key_marker_t newMarker;
        TRI_voc_size_t newMarkerSize = sizeof(TRI_doc_edge_key_marker_t);
            
        char* body = ((char*) oldMarker) + oldMarker->base._offsetJson;
        TRI_voc_size_t bodySize = oldMarker->base.base._size - sizeof(doc_edge_marker_t); 
        TRI_voc_size_t bodySizePadded = paddedSize - sizeof(doc_edge_marker_t); 
            
        char* keyBody;
        TRI_voc_size_t keyBodySize;
            
        size_t keySize;
        size_t toSize;
        size_t fromSize;            
           
        char* oldKey = (char*) oldMarker + oldMarker->base._offsetKey;
        char* toKey = (char*) oldMarker + oldMarker->_offsetToKey;
        char* fromKey = (char*) oldMarker + oldMarker->_offsetFromKey;
            
        keySize = strlen(oldKey) + 1;
        toSize = strlen(toKey) + 1;
        fromSize = strlen(fromKey) + 1;

        keyBodySize = TRI_DF_ALIGN_BLOCK(keySize + toSize + fromSize);            
        keyBody = (char*) TRI_Allocate(TRI_CORE_MEM_ZONE, keyBodySize, true);
            
        TRI_CopyString(keyBody,                    oldKey,  keySize);      
        TRI_CopyString(keyBody + keySize,          toKey,   toSize);      
        TRI_CopyString(keyBody + keySize + toSize, fromKey, fromSize);      
        
        TRI_sequence_value_t sequenceValue = TRI_SEQUENCE_VALUE(oldMarker->base._rid);
        newMarker.base._tid = sequenceValue;
        TRI_UpdateGlobalIdSequence(sequenceValue);

        newMarker.base._shape = oldMarker->base._shape;
        newMarker.base._offsetKey = newMarkerSize;
        newMarker.base._offsetJson = newMarkerSize + keyBodySize;
            
        newMarker._offsetToKey = newMarkerSize + keySize;
        newMarker._offsetFromKey = newMarkerSize + keySize + toSize;
        newMarker._toCid = oldMarker->_toCid;
        newMarker._fromCid = oldMarker->_fromCid;
        
        LOG_DEBUG("found edge marker, type: %d, key: '%s', fromCid: %llu, fromKey: %s, toCid: %llu, toKey: %s, rid: %llu, size: %d, crc: %lu", 
                  (int) oldMarker->base.base._type, 
                  oldKey, 
                  oldMarker->_fromCid,
                  fromKey,
                  oldMarker->_toCid,
                  toKey,
                  (unsigned long long) oldMarker->base._rid,
                  (int) oldMarker->base.base._size,
                  (unsigned long) oldMarker->base.base._crc);
            
        TRI_InitMarkerDatafile(&newMarker.base.base, newMarkerSize + keyBodySize + bodySize, TRI_DOC_MARKER_KEY_EDGE, serverId, sequenceValue);
        TRI_FillCrcKeyMarkerDatafile(df, &newMarker.base.base, newMarkerSize, keyBody, keyBodySize, body, bodySize);

        ssize_t writeResult;
        writeResult = TRI_WRITE(fdout, &newMarker, newMarkerSize);
        if (writeResult == 0) {
          *err = TRI_ERROR_INTERNAL;
        }
        writeResult = TRI_WRITE(fdout, keyBody, keyBodySize);
        if (writeResult == 0) {
          *err = TRI_ERROR_INTERNAL;
        }
        writeResult = TRI_WRITE(fdout, body, bodySizePadded);
        if (writeResult == 0) {
          *err = TRI_ERROR_INTERNAL;
        }

        TRI_Free(TRI_CORE_MEM_ZONE, keyBody);

        return newMarkerSize + keyBodySize + bodySizePadded;
      }

////////////////////////////////////////////////////////////////////////////////
/// @brief convert a 1.1 deletion marker
////////////////////////////////////////////////////////////////////////////////

      int64_t convertDeletionMarker (char* payload, 
                                     const off_t paddedSize, 
                                     int* err,
                                     TRI_datafile_t* df, 
                                     int fdout, 
                                     const TRI_server_id_t serverId) {
        *err = TRI_ERROR_NO_ERROR;

        doc_deletion_marker_t* oldMarker = (doc_deletion_marker_t*) payload;                        

        TRI_doc_deletion_key_marker_t newMarker;
        TRI_voc_size_t newMarkerSize = sizeof(TRI_doc_deletion_key_marker_t);
            
        TRI_voc_size_t keyBodySize; 
        char* keyBody;
        TRI_voc_size_t keySize;

        char* oldKey = (char*) oldMarker + oldMarker->_offsetKey;

        keySize = strlen(oldKey) + 1;
        keyBodySize = TRI_DF_ALIGN_BLOCK(keySize);
        keyBody = (char*) TRI_Allocate(TRI_CORE_MEM_ZONE, keyBodySize, true);
        TRI_CopyString(keyBody, oldKey, keySize);      

        TRI_sequence_value_t sequenceValue = TRI_SEQUENCE_VALUE(oldMarker->_rid);
        newMarker._tid = sequenceValue;
        TRI_UpdateGlobalIdSequence(sequenceValue);
        
        LOG_DEBUG("found deletion marker, type: %d, key: '%s', rid: %llu", 
                  (int) oldMarker->base._type, 
                  oldKey,
                  (unsigned long long) oldMarker->_rid);
        
        TRI_InitMarkerDatafile(&newMarker.base, newMarkerSize + keyBodySize, TRI_DOC_MARKER_KEY_DELETION, serverId, sequenceValue);
        TRI_FillCrcKeyMarkerDatafile(df, &newMarker.base, newMarkerSize, keyBody, keyBodySize, NULL, 0);
        
        LOG_DEBUG("new deletion marker, type: %d, size: %d, serverId: %llu, sequenceValue: %llu, key: '%s', tid: %llu, crc: %lu", 
                  (int) newMarker.base._type, 
                  (int) newMarker.base._size, 
                  (unsigned long long) serverId,
                  (unsigned long long) sequenceValue,
                  keyBody,
                  (unsigned long long) newMarker._tid,
                  (unsigned long) newMarker.base._crc);
            
        ssize_t writeResult;
        writeResult = TRI_WRITE(fdout, &newMarker, newMarkerSize);
        if (writeResult == 0) {
          *err = TRI_ERROR_INTERNAL;
        }
        writeResult = TRI_WRITE(fdout, (char*) keyBody, keyBodySize);
        if (writeResult == 0) {
          *err = TRI_ERROR_INTERNAL;
        }

        TRI_Free(TRI_CORE_MEM_ZONE, keyBody);
        
        return newMarker.base._size;
      }

////////////////////////////////////////////////////////////////////////////////
/// @}
////////////////////////////////////////////////////////////////////////////////

    }
  }
}


#endif

// Local Variables:
// mode: outline-minor
// outline-regexp: "^\\(/// @brief\\|/// {@inheritDoc}\\|/// @addtogroup\\|/// @page\\|// --SECTION--\\|/// @\\}\\)"
// End:

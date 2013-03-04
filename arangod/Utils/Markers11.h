////////////////////////////////////////////////////////////////////////////////
/// @brief marker types from ArangoDB 1.1
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

#ifndef TRIAGENS_ARANGOD_UTILS_MARKERS11_H
#define TRIAGENS_ARANGOD_UTILS_MARKERS11_H 1

#include "VocBase/datafile.h"
#include "VocBase/vocbase.h"

// -----------------------------------------------------------------------------
// --SECTION--                                           Arango 1.1 marker types
// -----------------------------------------------------------------------------

namespace triagens {
  namespace arango {
    namespace markers11 {

// -----------------------------------------------------------------------------
// --SECTION--                                                      public types
// -----------------------------------------------------------------------------

////////////////////////////////////////////////////////////////////////////////
/// @addtogroup VocBase
/// @{
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
/// @brief document id type used in 1.1
////////////////////////////////////////////////////////////////////////////////

      typedef uint64_t voc_did_t;

////////////////////////////////////////////////////////////////////////////////
/// @brief base marker used in 1.1
////////////////////////////////////////////////////////////////////////////////

      typedef struct {
        TRI_voc_size_t _size;                 // 4 bytes, must be supplied
        TRI_voc_crc_t _crc;                   // 4 bytes, will be generated

        TRI_df_marker_type_t _type;           // 4 bytes, must be supplied

#ifdef TRI_PADDING_32
        char _padding_df_marker[4];
#endif

        uint64_t _tick;
      }
      base_marker_t;

////////////////////////////////////////////////////////////////////////////////
/// @brief document marker used in 1.1
////////////////////////////////////////////////////////////////////////////////

      typedef struct {
        base_marker_t base;

        voc_did_t _did;        // this is the tick for a create, but not an update
        TRI_voc_rid_t _rid;    // this is the tick for an create and update
        TRI_voc_eid_t _sid;

        TRI_shape_sid_t _shape;
      }
      doc_document_marker_t;

////////////////////////////////////////////////////////////////////////////////
/// @brief edge marker used in 1.1
////////////////////////////////////////////////////////////////////////////////

      typedef struct {
        doc_document_marker_t base;

        TRI_voc_cid_t _toCid;
        voc_did_t _toDid;

        TRI_voc_cid_t _fromCid;
        voc_did_t _fromDid;
      }
      doc_edge_marker_t;

////////////////////////////////////////////////////////////////////////////////
/// @brief deletion marker used in 1.1
////////////////////////////////////////////////////////////////////////////////

      typedef struct {
        base_marker_t base;

        voc_did_t _did;        // this is the tick for a create, but not an update
        TRI_voc_rid_t _rid;    // this is the tick for an create and update
        TRI_voc_eid_t _sid;
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
/// @brief convert a 1.1 document marker
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

        char* body = ((char*) oldMarker) + sizeof(doc_document_marker_t);
        TRI_voc_size_t bodySize = oldMarker->base._size - sizeof(doc_document_marker_t); 
        TRI_voc_size_t bodySizePadded = paddedSize - sizeof(doc_document_marker_t); 
            
        char* keyBody;
        TRI_voc_size_t keyBodySize; 
        TRI_voc_size_t keySize;

        char didBuffer[33];  
        memset(&newMarker, 0, newMarkerSize); 
        sprintf(didBuffer, "%llu", (unsigned long long) oldMarker->_did);

        keySize = strlen(didBuffer) + 1;
        keyBodySize = TRI_DF_ALIGN_BLOCK(keySize);
        keyBody = (char*) TRI_Allocate(TRI_CORE_MEM_ZONE, keyBodySize, true);
        TRI_CopyString(keyBody, didBuffer, keySize);      

        newMarker._rid = oldMarker->_rid;
        newMarker._sid = oldMarker->_sid;
        newMarker._shape = oldMarker->_shape;
        newMarker._offsetKey = newMarkerSize;
        newMarker._offsetJson = newMarkerSize + keyBodySize;
           
        TRI_InitMarkerDatafile(&newMarker.base, newMarkerSize + keyBodySize + bodySize, TRI_DOC_MARKER_KEY_DOCUMENT, serverId, TRI_SEQUENCE_VALUE(oldMarker->base._tick));
        TRI_FillCrcKeyMarkerDatafile(df, &newMarker.base, newMarkerSize, keyBody, keyBodySize, body, bodySize);

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

        //LOG_INFO("found doc marker, type: '%d', did: '%d', rid: '%d', size: '%d', crc: '%d'", marker._type, oldMarker->_did, oldMarker->_rid,newMarker.base._size,newMarker.base._crc);

        TRI_Free(TRI_CORE_MEM_ZONE, keyBody);

        return sizeof(newMarker) + keyBodySize + bodySizePadded;
      }

////////////////////////////////////////////////////////////////////////////////
/// @brief convert a 1.1 edge marker
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
            
        char* body = ((char*) oldMarker) + sizeof(doc_edge_marker_t);
        TRI_voc_size_t bodySize = oldMarker->base.base._size - sizeof(doc_edge_marker_t); 
        TRI_voc_size_t bodySizePadded = paddedSize - sizeof(doc_edge_marker_t); 
            
        char* keyBody;
        TRI_voc_size_t keyBodySize;
            
        size_t keySize;
        size_t toSize;
        size_t fromSize;            
            
        char didBuffer[33];  
        char toDidBuffer[33];  
        char fromDidBuffer[33];  

        memset(&newMarker, 0, newMarkerSize); 
        sprintf(didBuffer,"%llu", (unsigned long long) oldMarker->base._did);
        sprintf(toDidBuffer,"%llu", (unsigned long long) oldMarker->_toDid);
        sprintf(fromDidBuffer,"%llu", (unsigned long long) oldMarker->_fromDid);
            
        keySize = strlen(didBuffer) + 1;
        toSize = strlen(toDidBuffer) + 1;
        fromSize = strlen(fromDidBuffer) + 1;

        keyBodySize = TRI_DF_ALIGN_BLOCK(keySize + toSize + fromSize);            
        keyBody = (char*) TRI_Allocate(TRI_CORE_MEM_ZONE, keyBodySize, true);
            
        TRI_CopyString(keyBody,                    didBuffer,     keySize);      
        TRI_CopyString(keyBody + keySize,          toDidBuffer,   toSize);      
        TRI_CopyString(keyBody + keySize + toSize, fromDidBuffer, fromSize);      

        newMarker.base._rid = oldMarker->base._rid;
        newMarker.base._sid = oldMarker->base._sid;                        
        newMarker.base._shape = oldMarker->base._shape;
        newMarker.base._offsetKey = newMarkerSize;
        newMarker.base._offsetJson = newMarkerSize + keyBodySize;
            
        newMarker._offsetToKey = newMarkerSize + keySize;
        newMarker._offsetFromKey = newMarkerSize + keySize + toSize;
        newMarker._toCid = oldMarker->_toCid;
        newMarker._fromCid = oldMarker->_fromCid;
            
        TRI_InitMarkerDatafile(&newMarker.base.base, newMarkerSize + keyBodySize + bodySize, TRI_DOC_MARKER_KEY_EDGE, serverId, TRI_SEQUENCE_VALUE(oldMarker->base.base._tick));
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

        //LOG_INFO("found edge marker, type: '%d', did: '%d', rid: '%d', size: '%d', crc: '%d'", marker._type, oldMarker->base._did, oldMarker->base._rid,newMarker.base.base._size,newMarker.base.base._crc);

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

        char didBuffer[33];  
        memset(&newMarker, 0, newMarkerSize); 
        sprintf(didBuffer, "%llu", (unsigned long long) oldMarker->_did);
      
        keySize = strlen(didBuffer) + 1;
        keyBodySize = TRI_DF_ALIGN_BLOCK(keySize);
        keyBody = (char*) TRI_Allocate(TRI_CORE_MEM_ZONE, keyBodySize, true);
        TRI_CopyString(keyBody, didBuffer, keySize);      

        newMarker._rid = oldMarker->_rid;
        newMarker._sid = oldMarker->_sid;
        newMarker._offsetKey = newMarkerSize;
            
        TRI_InitMarkerDatafile(&newMarker.base, newMarkerSize + keyBodySize, TRI_DOC_MARKER_KEY_DELETION, serverId, TRI_SEQUENCE_VALUE(oldMarker->base._tick));
        TRI_FillCrcKeyMarkerDatafile(df, &newMarker.base, newMarkerSize, keyBody, keyBodySize, NULL, 0);

        ssize_t writeResult;
        writeResult = TRI_WRITE(fdout, &newMarker, newMarkerSize);
        if (writeResult == 0) {
          *err = TRI_ERROR_INTERNAL;
        }
        writeResult = TRI_WRITE(fdout, (char*) keyBody, keyBodySize);
        if (writeResult == 0) {
          *err = TRI_ERROR_INTERNAL;
        }

        //LOG_INFO("found deletion marker, type: '%d', did: '%d', rid: '%d'", marker._type, oldMarker->_did, oldMarker->_rid);

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

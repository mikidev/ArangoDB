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
// --SECTION--                                                      public types
// -----------------------------------------------------------------------------

////////////////////////////////////////////////////////////////////////////////
/// @addtogroup VocBase
/// @{
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
/// @brief marker types used in ArangoDB 1.1
////////////////////////////////////////////////////////////////////////////////

namespace triagens {
  namespace arango {
    namespace markers11 {

      typedef uint64_t voc_did_t;


      typedef struct {
        TRI_voc_size_t _size;                 // 4 bytes, must be supplied
        TRI_voc_crc_t _crc;                   // 4 bytes, will be generated

        TRI_df_marker_type_t _type;           // 4 bytes, must be supplied

#ifdef TRI_PADDING_32
        char _padding_df_marker[4];
#endif

        uint64_t _tick;
      }
      base_marker_t_deprecated;


      typedef struct {
        base_marker_t_deprecated base;

        voc_did_t _did;        // this is the tick for a create, but not an update
        TRI_voc_rid_t _rid;    // this is the tick for an create and update
        TRI_voc_eid_t _sid;

        TRI_shape_sid_t _shape;
      }
      doc_document_marker_t_deprecated;


      typedef struct {
        doc_document_marker_t_deprecated base;

        TRI_voc_cid_t _toCid;
        voc_did_t _toDid;

        TRI_voc_cid_t _fromCid;
        voc_did_t _fromDid;
      }
      doc_edge_marker_t_deprecated;


      typedef struct {
        base_marker_t_deprecated base;

        voc_did_t _did;        // this is the tick for a create, but not an update
        TRI_voc_rid_t _rid;    // this is the tick for an create and update
        TRI_voc_eid_t _sid;
      }
      doc_deletion_marker_t_deprecated;

    }
  }
}

////////////////////////////////////////////////////////////////////////////////
/// @}
////////////////////////////////////////////////////////////////////////////////

#endif

// Local Variables:
// mode: outline-minor
// outline-regexp: "^\\(/// @brief\\|/// {@inheritDoc}\\|/// @addtogroup\\|/// @page\\|// --SECTION--\\|/// @\\}\\)"
// End:

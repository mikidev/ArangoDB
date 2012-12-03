////////////////////////////////////////////////////////////////////////////////
/// @brief full text search
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
/// @author R. A. Parker  
/// @author Copyright 2012, triagens GmbH, Cologne, Germany
////////////////////////////////////////////////////////////////////////////////

#ifndef TRIAGENS_FULLTEXT_FTS_INDEX_H
#define TRIAGENS_FULLTEXT_FTS_INDEX_H 1

#include "BasicsC/common.h"

#ifdef __cplusplus
extern "C" {
#endif

// -----------------------------------------------------------------------------
// --SECTION--                                                      public types
// -----------------------------------------------------------------------------

////////////////////////////////////////////////////////////////////////////////
/// @addtogroup Fulltext
/// @{
////////////////////////////////////////////////////////////////////////////////

typedef struct FTS_index_s {
}
FTS_index_t;

////////////////////////////////////////////////////////////////////////////////
/// @brief index options
////////////////////////////////////////////////////////////////////////////////

#define FTS_INDEX_SUBSTRINGS 1

////////////////////////////////////////////////////////////////////////////////
/// @brief default index sizes
////////////////////////////////////////////////////////////////////////////////

//#define FTS_SIZES_DEFAULT {10,1000,57,100,0,0,0,0,0,0}

////////////////////////////////////////////////////////////////////////////////
/// @brief type of collections indexed
////////////////////////////////////////////////////////////////////////////////

typedef uint64_t FTS_collection_id_t;

////////////////////////////////////////////////////////////////////////////////
/// @brief type of documents indexed
////////////////////////////////////////////////////////////////////////////////

typedef uint64_t FTS_document_id_t;

////////////////////////////////////////////////////////////////////////////////
/// @brief fulltext query specification
////////////////////////////////////////////////////////////////////////////////

typedef struct {
  size_t     _len;     // number of words in the query
  uint8_t**  _texts;   // the actual words we are looking for
  int*       _localOptions; // options for the words we are looking for
} 
FTS_query_t;

////////////////////////////////////////////////////////////////////////////////
/// @brief query options
////////////////////////////////////////////////////////////////////////////////

#define FTS_MATCH_COMPLETE   1
#define FTS_MATCH_PREFIX     2
#define FTS_MATCH_SUBSTRING  4

////////////////////////////////////////////////////////////////////////////////
/// @brief query result list
////////////////////////////////////////////////////////////////////////////////

typedef struct {
  size_t             _len;
  FTS_document_id_t* _docs;
} 
FTS_document_ids_t;

////////////////////////////////////////////////////////////////////////////////
/// @brief type used to hold words for a document (passed by ArangoDB to the
/// index)
////////////////////////////////////////////////////////////////////////////////

typedef struct {
  size_t    _len;
  uint8_t** _texts;
  void (*free)(void*);
} 
FTS_texts_t;

////////////////////////////////////////////////////////////////////////////////
/// @}
////////////////////////////////////////////////////////////////////////////////

// -----------------------------------------------------------------------------
// --SECTION--                                                  public functions
// -----------------------------------------------------------------------------

////////////////////////////////////////////////////////////////////////////////
/// @addtogroup Fulltext
/// @{
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
/// @brief create a new fulltext index
////////////////////////////////////////////////////////////////////////////////

FTS_index_t* FTS_CreateIndex (FTS_collection_id_t,
                              void*,
                              FTS_texts_t* (*getTexts)(FTS_collection_id_t, FTS_document_id_t, void*),
                              int options, 
                              uint64_t sizes[10]);

////////////////////////////////////////////////////////////////////////////////
/// @brief free an existing fulltext index
////////////////////////////////////////////////////////////////////////////////

void FTS_FreeIndex (FTS_index_t*);

////////////////////////////////////////////////////////////////////////////////
/// @brief add a document to the index
////////////////////////////////////////////////////////////////////////////////

void FTS_AddDocument (FTS_index_t*, FTS_document_id_t);

////////////////////////////////////////////////////////////////////////////////
/// @brief delete a document from the index
////////////////////////////////////////////////////////////////////////////////

void FTS_DeleteDocument (FTS_index_t*, FTS_document_id_t);

////////////////////////////////////////////////////////////////////////////////
/// @brief update an existing document in the index
////////////////////////////////////////////////////////////////////////////////

void FTS_UpdateDocument (FTS_index_t*, FTS_document_id_t);

////////////////////////////////////////////////////////////////////////////////
/// @brief perform a search in the index
////////////////////////////////////////////////////////////////////////////////

FTS_document_ids_t* FTS_FindDocuments (FTS_index_t*, FTS_query_t*);

////////////////////////////////////////////////////////////////////////////////
/// @brief free results of a search
////////////////////////////////////////////////////////////////////////////////

void FTS_Free_Documents (FTS_document_ids_t*);

////////////////////////////////////////////////////////////////////////////////
/// @}
////////////////////////////////////////////////////////////////////////////////

#ifdef __cplusplus
}
#endif

#endif

// Local Variables:
// mode: outline-minor
// outline-regexp: "^\\(/// @brief\\|/// {@inheritDoc}\\|/// @addtogroup\\|// --SECTION--\\|/// @\\}\\)"
// End:

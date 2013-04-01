////////////////////////////////////////////////////////////////////////////////
/// @brief primary collection
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
/// @author Dr. Frank Celler
/// @author Copyright 2011-2013, triAGENS GmbH, Cologne, Germany
////////////////////////////////////////////////////////////////////////////////

#include "primary-collection.h"

#include "BasicsC/conversions.h"
#include "BasicsC/files.h"
#include "BasicsC/hashes.h"
#include "BasicsC/logging.h"
#include "BasicsC/tri-strings.h"

#include "VocBase/key-generator.h"
#include "VocBase/voc-shaper.h"

// -----------------------------------------------------------------------------
// --SECTION--                                                 private functions
// -----------------------------------------------------------------------------

////////////////////////////////////////////////////////////////////////////////
/// @addtogroup VocBase
/// @{
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
/// @brief hashs the document id
////////////////////////////////////////////////////////////////////////////////

static uint64_t HashKeyHeader (TRI_associative_pointer_t* array, void const* key) {
  return TRI_FnvHashString((char const*) key);
}

////////////////////////////////////////////////////////////////////////////////
/// @brief hashs the document header
////////////////////////////////////////////////////////////////////////////////

static uint64_t HashElementDocument (TRI_associative_pointer_t* array, void const* element) {
  TRI_doc_mptr_t const* e = element;
  return TRI_FnvHashString((char const*) e->_key);
}

////////////////////////////////////////////////////////////////////////////////
/// @brief compares a document id and a document
////////////////////////////////////////////////////////////////////////////////

static bool IsEqualKeyDocument (TRI_associative_pointer_t* array, void const* key, void const* element) {
  TRI_doc_mptr_t const* e = element;

  char const * k = key;
  return (strcmp(k, e->_key) == 0);
}

////////////////////////////////////////////////////////////////////////////////
/// @brief hashs a datafile identifier
////////////////////////////////////////////////////////////////////////////////

static uint64_t HashKeyDatafile (TRI_associative_pointer_t* array, void const* key) {
  TRI_voc_tick_t const* k = key;

  return *k;
}

////////////////////////////////////////////////////////////////////////////////
/// @brief hashs a datafile identifier
////////////////////////////////////////////////////////////////////////////////

static uint64_t HashElementDatafile (TRI_associative_pointer_t* array, void const* element) {
  TRI_doc_datafile_info_t const* e = element;

  return e->_fid;
}

////////////////////////////////////////////////////////////////////////////////
/// @brief compares a datafile identifier and a datafile info
////////////////////////////////////////////////////////////////////////////////

static bool IsEqualKeyElementDatafile (TRI_associative_pointer_t* array, void const* key, void const* element) {
  TRI_voc_tick_t const* k = key;
  TRI_doc_datafile_info_t const* e = element;

  return *k == e->_fid;
}

////////////////////////////////////////////////////////////////////////////////
/// @brief creates a journal or a compactor journal
////////////////////////////////////////////////////////////////////////////////

static TRI_datafile_t* CreateJournal (TRI_primary_collection_t* primary, bool compactor) {
  TRI_col_header_marker_t cm;
  TRI_collection_t* collection;
  TRI_datafile_t* journal;
  TRI_df_marker_t* position;
  int res;
  char* filename;

  collection = &primary->base;

  if (collection->_info._isVolatile) {
    // in-memory collection
    filename = NULL;
  }
  else {
    char* jname;
    char* number;

    // construct a suitable filename
    number = TRI_StringUInt64(TRI_NewTickVocBase());

    if (compactor) {
      jname = TRI_Concatenate3String("journal-", number, ".db");
    }
    else {
      jname = TRI_Concatenate3String("compactor-", number, ".db");
    }

    filename = TRI_Concatenate2File(collection->_directory, jname);

    TRI_FreeString(TRI_CORE_MEM_ZONE, number);
    TRI_FreeString(TRI_CORE_MEM_ZONE, jname);
  }

  // create journal file
  journal = TRI_CreateDatafile(filename, collection->_info._maximalSize);

  if (filename != NULL) {
    TRI_FreeString(TRI_CORE_MEM_ZONE, filename);
  }

  if (journal == NULL) {
    if (TRI_errno() == TRI_ERROR_OUT_OF_MEMORY_MMAP) {
      collection->_lastError = TRI_set_errno(TRI_ERROR_OUT_OF_MEMORY_MMAP);
      collection->_state = TRI_COL_STATE_READ;
    }
    else {
      collection->_lastError = TRI_set_errno(TRI_ERROR_ARANGO_NO_JOURNAL);
      collection->_state = TRI_COL_STATE_WRITE_ERROR;
    }

    return NULL;
  }

  LOG_TRACE("created a new primary journal '%s'", journal->getName(journal));


  if (journal->isPhysical(journal)) {
    char* jname;
    char* number;
    bool ok;

    // and use the correct name
    number = TRI_StringUInt64(journal->_fid);

    if (compactor) {
      jname = TRI_Concatenate3String("compactor-", number, ".db");
    }
    else {
      jname = TRI_Concatenate3String("journal-", number, ".db");
    }

    filename = TRI_Concatenate2File(collection->_directory, jname);

    TRI_FreeString(TRI_CORE_MEM_ZONE, number);
    TRI_FreeString(TRI_CORE_MEM_ZONE, jname);

    ok = TRI_RenameDatafile(journal, filename);

    if (! ok) {
      LOG_WARNING("failed to rename the journal to '%s': %s", filename, TRI_last_error());
    }
    else {
      LOG_TRACE("renamed journal to '%s'", filename);
    }

    TRI_FreeString(TRI_CORE_MEM_ZONE, filename);
  }


  // create a collection header
  res = TRI_ReserveElementDatafile(journal, sizeof(TRI_col_header_marker_t), &position);

  if (res != TRI_ERROR_NO_ERROR) {
    collection->_lastError = journal->_lastError;
    LOG_ERROR("cannot create document header in journal '%s': %s", journal->getName(journal), TRI_last_error());

    TRI_FreeDatafile(journal);

    return NULL;
  }

  memset(&cm, 0, sizeof(cm));

  cm.base._size = sizeof(TRI_col_header_marker_t);
  cm.base._type = TRI_COL_MARKER_HEADER;
  cm.base._tick = TRI_NewTickVocBase();

  cm._cid = collection->_info._cid;

  TRI_FillCrcMarkerDatafile(journal, &cm.base, sizeof(cm), 0, 0, 0, 0);

  res = TRI_WriteElementDatafile(journal, position, &cm.base, sizeof(cm), 0, 0, 0, 0, true);

  if (res != TRI_ERROR_NO_ERROR) {
    collection->_lastError = journal->_lastError;
    LOG_ERROR("cannot create document header in journal '%s': %s", journal->getName(journal), TRI_last_error());

    TRI_FreeDatafile(journal);

    return NULL;
  }

  // that's it
  if (compactor) {
    TRI_PushBackVectorPointer(&collection->_compactors, journal);
  }
  else {
    TRI_PushBackVectorPointer(&collection->_journals, journal);
  }

  return journal;
}

////////////////////////////////////////////////////////////////////////////////
/// @brief closes a journal
///
/// Note that the caller must hold a lock protecting the _datafiles and
/// _journals entry.
////////////////////////////////////////////////////////////////////////////////

static bool CloseJournalPrimaryCollection (TRI_primary_collection_t* primary,
                                           size_t position,
                                           bool compactor) {
  TRI_datafile_t* journal;
  TRI_collection_t* collection;
  TRI_vector_pointer_t* vector;
  int res;

  collection = &primary->base;

  // either use a journal or a compactor
  if (compactor) {
    vector = &collection->_compactors;
  }
  else {
    vector = &collection->_journals;
  }

  // no journal at this position
  if (vector->_length <= position) {
    TRI_set_errno(TRI_ERROR_ARANGO_NO_JOURNAL);
    return false;
  }

  // seal and rename datafile
  journal = vector->_buffer[position];
  res = TRI_SealDatafile(journal);

  if (res != TRI_ERROR_NO_ERROR) {
    LOG_ERROR("failed to seal datafile '%s': %s", journal->getName(journal), TRI_last_error());

    TRI_RemoveVectorPointer(vector, position);
    TRI_PushBackVectorPointer(&collection->_datafiles, journal);

    return false;
  }

  if (journal->isPhysical(journal)) {
    // rename the file
    char* dname;
    char* filename;
    char* number;
    bool ok;

    number = TRI_StringUInt64(journal->_fid);
    dname = TRI_Concatenate3String("datafile-", number, ".db");
    filename = TRI_Concatenate2File(collection->_directory, dname);

    TRI_FreeString(TRI_CORE_MEM_ZONE, dname);
    TRI_FreeString(TRI_CORE_MEM_ZONE, number);

    ok = TRI_RenameDatafile(journal, filename);

    if (! ok) {
      LOG_ERROR("failed to rename datafile '%s' to '%s': %s", journal->getName(journal), filename, TRI_last_error());

      TRI_RemoveVectorPointer(vector, position);
      TRI_PushBackVectorPointer(&collection->_datafiles, journal);
      TRI_FreeString(TRI_CORE_MEM_ZONE, filename);

      return false;
    }

    TRI_FreeString(TRI_CORE_MEM_ZONE, filename);

    LOG_TRACE("closed journal '%s'", journal->getName(journal));
  }

  TRI_RemoveVectorPointer(vector, position);
  TRI_PushBackVectorPointer(&collection->_datafiles, journal);

  return true;
}

////////////////////////////////////////////////////////////////////////////////
/// @brief free an assoc array of datafile infos
////////////////////////////////////////////////////////////////////////////////

static void FreeDatafileInfo (TRI_associative_pointer_t* const files) {
  size_t i;
  size_t n;

  n = files->_nrAlloc;
  for (i = 0; i < n; ++i) {
    TRI_doc_datafile_info_t* file = files->_table[i];
    if (!file) {
      continue;
    }

    TRI_Free(TRI_UNKNOWN_MEM_ZONE, file);
  }

  TRI_DestroyAssociativePointer(files);
}

////////////////////////////////////////////////////////////////////////////////
/// @brief returns information about the collection
/// note: the collection lock must be held when calling this function
////////////////////////////////////////////////////////////////////////////////

static TRI_doc_collection_info_t* Figures (TRI_primary_collection_t* primary) {
  TRI_doc_collection_info_t* info;
  TRI_collection_t* base;
  size_t i;

  // prefill with 0's to init counters
  info = TRI_Allocate(TRI_UNKNOWN_MEM_ZONE, sizeof(TRI_doc_collection_info_t), true);

  if (info == NULL) {
    return NULL;
  }

  for (i = 0;  i < primary->_datafileInfo._nrAlloc;  ++i) {
    TRI_doc_datafile_info_t* d = primary->_datafileInfo._table[i];

    if (d != NULL) {
      info->_numberAlive += d->_numberAlive;
      info->_numberDead += d->_numberDead;
      info->_sizeAlive += d->_sizeAlive;
      info->_sizeDead += d->_sizeDead;
      info->_numberDeletion += d->_numberDeletion;
    }
  }

  // add the file sizes for datafiles and journals
  base = &primary->base;
  for (i = 0; i < base->_datafiles._length; ++i) {
    TRI_datafile_t* df = (TRI_datafile_t*) base->_datafiles._buffer[i];

    info->_datafileSize += df->_maximalSize;
    ++info->_numberDatafiles;
  }

  for (i = 0; i < base->_journals._length; ++i) {
    TRI_datafile_t* df = (TRI_datafile_t*) base->_journals._buffer[i];

    info->_journalfileSize += df->_maximalSize;
    ++info->_numberJournalfiles;
  }

  info->_numberShapes = (TRI_voc_ssize_t) primary->_shaper->numShapes(primary->_shaper);

  return info;
}

////////////////////////////////////////////////////////////////////////////////
/// @brief size of a primary collection
///
/// the caller must have read-locked the collection!
////////////////////////////////////////////////////////////////////////////////

static TRI_voc_size_t Count (TRI_primary_collection_t* primary) {
  TRI_doc_mptr_t const* mptr;
  TRI_voc_size_t result;
  void** end;
  void** ptr;

  ptr = primary->_primaryIndex._table;
  end = ptr + primary->_primaryIndex._nrAlloc;
  result = 0;

  for (;  ptr < end;  ++ptr) {
    if (*ptr != NULL) {
      mptr = *ptr;

      if (mptr->_validTo == 0) {
        ++result;
      }
    }
  }

  return result;
}

////////////////////////////////////////////////////////////////////////////////
/// @}
////////////////////////////////////////////////////////////////////////////////

// -----------------------------------------------------------------------------
// --SECTION--                                      constructors and destructors
// -----------------------------------------------------------------------------

////////////////////////////////////////////////////////////////////////////////
/// @addtogroup VocBase
/// @{
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
/// @brief initialises a primary collection
////////////////////////////////////////////////////////////////////////////////

int TRI_InitPrimaryCollection (TRI_primary_collection_t* primary,
                               TRI_shaper_t* shaper) {
  primary->_shaper = shaper;
  primary->_capConstraint = NULL;
  primary->_keyGenerator = NULL;

  primary->figures = Figures;
  primary->size    = Count;

  TRI_InitBarrierList(&primary->_barrierList, primary);

  TRI_InitAssociativePointer(&primary->_datafileInfo,
                             TRI_UNKNOWN_MEM_ZONE,
                             HashKeyDatafile,
                             HashElementDatafile,
                             IsEqualKeyElementDatafile,
                             NULL);

  TRI_InitAssociativePointer(&primary->_primaryIndex,
                             TRI_UNKNOWN_MEM_ZONE,
                             HashKeyHeader,
                             HashElementDocument,
                             IsEqualKeyDocument,
                             0);

  TRI_InitReadWriteLock(&primary->_lock);

  return TRI_ERROR_NO_ERROR;
}

////////////////////////////////////////////////////////////////////////////////
/// @brief destroys a primary collection
////////////////////////////////////////////////////////////////////////////////

void TRI_DestroyPrimaryCollection (TRI_primary_collection_t* primary) {
  if (primary->_keyGenerator != NULL) {
    TRI_FreeKeyGenerator(primary->_keyGenerator);
  }

  TRI_DestroyReadWriteLock(&primary->_lock);
  TRI_DestroyAssociativePointer(&primary->_primaryIndex);

  if (primary->_shaper != NULL) {
    TRI_FreeVocShaper(primary->_shaper);
  }

  FreeDatafileInfo(&primary->_datafileInfo);
  TRI_DestroyBarrierList(&primary->_barrierList);

  TRI_DestroyCollection(&primary->base);
}

////////////////////////////////////////////////////////////////////////////////
/// @}
////////////////////////////////////////////////////////////////////////////////

// -----------------------------------------------------------------------------
// --SECTION--                                               protected functions
// -----------------------------------------------------------------------------

////////////////////////////////////////////////////////////////////////////////
/// @addtogroup VocBase
/// @{
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
/// @brief finds a datafile description
////////////////////////////////////////////////////////////////////////////////

TRI_doc_datafile_info_t* TRI_FindDatafileInfoPrimaryCollection (TRI_primary_collection_t* primary,
                                                                TRI_voc_fid_t fid) {
  TRI_doc_datafile_info_t const* found;
  TRI_doc_datafile_info_t* dfi;

  found = TRI_LookupByKeyAssociativePointer(&primary->_datafileInfo, &fid);

  if (found != NULL) {
    union { TRI_doc_datafile_info_t const* c; TRI_doc_datafile_info_t* v; } cnv;

    cnv.c = found;
    return cnv.v;
  }

  dfi = TRI_Allocate(TRI_UNKNOWN_MEM_ZONE, sizeof(TRI_doc_datafile_info_t), true);

  if (dfi == NULL) {
    return NULL;
  }

  dfi->_fid = fid;

  TRI_InsertKeyAssociativePointer(&primary->_datafileInfo, &fid, dfi, true);

  return dfi;
}

////////////////////////////////////////////////////////////////////////////////
/// @brief creates a journal
///
/// Note that the caller must hold a lock protecting the _journals entry.
////////////////////////////////////////////////////////////////////////////////

TRI_datafile_t* TRI_CreateJournalPrimaryCollection (TRI_primary_collection_t* primary) {
  return CreateJournal(primary, false);
}

////////////////////////////////////////////////////////////////////////////////
/// @brief closes a journal
///
/// Note that the caller must hold a lock protecting the _datafiles and
/// _journals entry.
////////////////////////////////////////////////////////////////////////////////

bool TRI_CloseJournalPrimaryCollection (TRI_primary_collection_t* primary,
                                    size_t position) {
  return CloseJournalPrimaryCollection(primary, position, false);
}

////////////////////////////////////////////////////////////////////////////////
/// @brief creates a new compactor file
///
/// Note that the caller must hold a lock protecting the _journals entry.
////////////////////////////////////////////////////////////////////////////////

TRI_datafile_t* TRI_CreateCompactorPrimaryCollection (TRI_primary_collection_t* primary) {
  return CreateJournal(primary, true);
}

////////////////////////////////////////////////////////////////////////////////
/// @brief closes an existing compactor file
///
/// Note that the caller must hold a lock protecting the _datafiles and
/// _journals entry.
////////////////////////////////////////////////////////////////////////////////

bool TRI_CloseCompactorPrimaryCollection (TRI_primary_collection_t* primary,
                                      size_t position) {
  return CloseJournalPrimaryCollection(primary, position, true);
}

////////////////////////////////////////////////////////////////////////////////
/// @brief initialise a new operation context
////////////////////////////////////////////////////////////////////////////////

void TRI_InitContextPrimaryCollection (TRI_doc_operation_context_t* const context,
                                       TRI_primary_collection_t* const primary,
                                       TRI_doc_update_policy_e policy,
                                       bool forceSync) {
  context->_collection = primary;
  context->_policy = policy;
  context->_expectedRid = 0;
  context->_previousRid = NULL;
  context->_sync = forceSync || primary->base._info._waitForSync;
  context->_allowRollback = true;
}

////////////////////////////////////////////////////////////////////////////////
/// @brief initialise a new operation context for reads
////////////////////////////////////////////////////////////////////////////////

void TRI_InitReadContextPrimaryCollection (TRI_doc_operation_context_t* const context,
                                           TRI_primary_collection_t* const primary) {
  context->_collection = primary;
  context->_policy = TRI_DOC_UPDATE_LAST_WRITE;
  context->_expectedRid = 0;
  context->_previousRid = NULL;
  context->_sync = false;
  context->_allowRollback = false;
}

////////////////////////////////////////////////////////////////////////////////
/// @brief compare revision of found document with revision specified in policy
/// this will also store the actual revision id found in the database in the
/// context variable _previousRid, but only if this is not NULL
////////////////////////////////////////////////////////////////////////////////

int TRI_RevisionCheck (const TRI_doc_operation_context_t* const context,
                       const TRI_voc_rid_t actualRid) {

  // store previous revision
  if (context->_previousRid != NULL) {
    *(context->_previousRid) = actualRid;
  }

  // check policy
  switch (context->_policy) {
    case TRI_DOC_UPDATE_ERROR:
      if (context->_expectedRid != 0 && context->_expectedRid != actualRid) {
        return TRI_ERROR_ARANGO_CONFLICT;
      }
      break;

    case TRI_DOC_UPDATE_CONFLICT:
      return TRI_ERROR_NOT_IMPLEMENTED;

    case TRI_DOC_UPDATE_ILLEGAL:
      return TRI_ERROR_INTERNAL;

    case TRI_DOC_UPDATE_LAST_WRITE:
      return TRI_ERROR_NO_ERROR;
  }

  return TRI_ERROR_NO_ERROR;
}

////////////////////////////////////////////////////////////////////////////////
/// @}
////////////////////////////////////////////////////////////////////////////////

// Local Variables:
// mode: outline-minor
// outline-regexp: "/// @brief\\|/// {@inheritDoc}\\|/// @addtogroup\\|/// @page\\|// --SECTION--\\|/// @\\}"
// End:

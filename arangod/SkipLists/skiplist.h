////////////////////////////////////////////////////////////////////////////////
/// @brief skip list implementation
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
/// @author Dr. O
/// @author Copyright 2006-2012, triAGENS GmbH, Cologne, Germany
////////////////////////////////////////////////////////////////////////////////


#ifndef TRIAGENS_BASICS_C_SKIPLIST_H
#define TRIAGENS_BASICS_C_SKIPLIST_H 1

#include "BasicsC/common.h"

#include "BasicsC/locks.h"
#include "BasicsC/vector.h"
#include "SkipLists/skiplistIndex.h"

#ifdef __cplusplus
extern "C" {
#endif

// -----------------------------------------------------------------------------
// --SECTION--                                              forward declarations
// -----------------------------------------------------------------------------

struct TRI_skiplist_node_s;

// -----------------------------------------------------------------------------
// --SECTION--                                             skiplist public types
// -----------------------------------------------------------------------------

////////////////////////////////////////////////////////////////////////////////
/// @addtogroup Skiplist
/// @{
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
/// @brief types which enumerate the probability used to determine the height of node
////////////////////////////////////////////////////////////////////////////////

typedef enum {
  TRI_SKIPLIST_PROB_HALF,
  TRI_SKIPLIST_PROB_THIRD,
  TRI_SKIPLIST_PROB_QUARTER
} 
TRI_skiplist_prob_e;  


typedef enum {
  TRI_SKIPLIST_COMPARE_STRICTLY_LESS = -1,
  TRI_SKIPLIST_COMPARE_STRICTLY_GREATER = 1,
  TRI_SKIPLIST_COMPARE_STRICTLY_EQUAL = 0,
  TRI_SKIPLIST_COMPARE_SLIGHTLY_LESS = -2,
  TRI_SKIPLIST_COMPARE_SLIGHTLY_GREATER = 2
} 
TRI_skiplist_compare_e;  

////////////////////////////////////////////////////////////////////////////////
/// @brief storage structure for a node's nearest neighbours
////////////////////////////////////////////////////////////////////////////////

typedef struct TRI_skiplist_nb_s {
  struct TRI_skiplist_node_s* _prev;
  struct TRI_skiplist_node_s* _next;
}
TRI_skiplist_nb_t; // nearest neighbour;

////////////////////////////////////////////////////////////////////////////////
/// @brief structure of a skip list node (unique and non-unique)
////////////////////////////////////////////////////////////////////////////////

typedef struct TRI_skiplist_node_s {
  TRI_skiplist_nb_t* _column; // these represent the levels
  uint32_t _colLength; 
  void* _extraData;
  TRI_skiplist_index_element_t _element;  
} 
TRI_skiplist_node_t;
  
////////////////////////////////////////////////////////////////////////////////
/// @brief The base structure of a skiplist (unique and non-unique)
////////////////////////////////////////////////////////////////////////////////

typedef struct TRI_skiplist_base_s {
  // ...........................................................................
  // The maximum height of this skip list. Thus 2^(_maxHeight) elements can be
  // stored in the skip list. 
  // ...........................................................................

  uint32_t _maxHeight;

  // ...........................................................................
  // The size of each element which is to be stored.
  // ...........................................................................

  uint32_t _elementSize;
  
  // ...........................................................................
  // The actual list itself
  // ...........................................................................

  char* _skiplist; 
  
  // ...........................................................................
  // The probability which is used to determine the level for insertions
  // into the list. Note the following
  // ...........................................................................

  TRI_skiplist_prob_e _prob;
  int32_t             _numRandom;
  uint32_t*           _random;
  
  
  TRI_skiplist_node_t _startNode;
  TRI_skiplist_node_t _endNode;
  
}
TRI_skiplist_base_t;

////////////////////////////////////////////////////////////////////////////////
/// @}
////////////////////////////////////////////////////////////////////////////////


// -----------------------------------------------------------------------------
// --SECTION--                                      unique skiplist public types
// -----------------------------------------------------------------------------

////////////////////////////////////////////////////////////////////////////////
/// @addtogroup Skiplist_unique
/// @{
////////////////////////////////////////////////////////////////////////////////



////////////////////////////////////////////////////////////////////////////////
/// @brief structure used for a skip list which only accepts unique entries
////////////////////////////////////////////////////////////////////////////////

typedef struct TRI_skiplist_s {
  TRI_skiplist_base_t _base;

  // ...........................................................................
  // callback compare function
  // < 0: implies left < right
  // == 0: implies left == right
  // > 0: implies left > right
  // ...........................................................................

  int (*compareElementElement) (struct TRI_skiplist_s*,
                                TRI_skiplist_index_element_t*,
                                TRI_skiplist_index_element_t*,
                                int);

  int (*compareKeyElement) (struct TRI_skiplist_s*,
                            TRI_skiplist_index_key_t*,
                            TRI_skiplist_index_element_t*,
                            int);    
}
TRI_skiplist_t;


////////////////////////////////////////////////////////////////////////////////
/// @brief structure used for a skip list which only accepts unique entries and is thread safe
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
// structure for a skiplist which allows unique entries -- with locking
// available for its nearest neighbours.
// TODO: implement locking for nearest neighbours rather than for all of index
////////////////////////////////////////////////////////////////////////////////

typedef struct TRI_skiplist_synced_s {
  TRI_skiplist_t _base;
  TRI_read_write_lock_t _lock;  
} TRI_skiplist_synced_t;


////////////////////////////////////////////////////////////////////////////////
/// @}
////////////////////////////////////////////////////////////////////////////////


// -----------------------------------------------------------------------------
// --SECTION--                 unique skiplist      constructors and destructors
// -----------------------------------------------------------------------------

////////////////////////////////////////////////////////////////////////////////
/// @addtogroup Skiplist_unique
/// @{
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
/// @brief initialises a skip list
////////////////////////////////////////////////////////////////////////////////

void TRI_InitSkipList (TRI_skiplist_t*,
                       TRI_skiplist_prob_e,
                       uint32_t);

////////////////////////////////////////////////////////////////////////////////
/// @brief destroys a skip list, but does not free the pointer
////////////////////////////////////////////////////////////////////////////////

void TRI_DestroySkipList (TRI_skiplist_t*);

////////////////////////////////////////////////////////////////////////////////
/// @brief destroys a skip list and frees the pointer
////////////////////////////////////////////////////////////////////////////////

void TRI_FreeSkipList (TRI_skiplist_t*);

////////////////////////////////////////////////////////////////////////////////
/// @}
////////////////////////////////////////////////////////////////////////////////

// -----------------------------------------------------------------------------
// --SECTION--                                 unique skiplist  public functions
// -----------------------------------------------------------------------------

////////////////////////////////////////////////////////////////////////////////
/// @addtogroup Skiplist_unique
/// @{
////////////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////////////
/// @brief returns the end node which belongs to a skiplist
////////////////////////////////////////////////////////////////////////////////

TRI_skiplist_node_t* TRI_EndNodeSkipList (TRI_skiplist_t* skiplist);

////////////////////////////////////////////////////////////////////////////////
/// @brief adds an element to the skip list using key for comparison
////////////////////////////////////////////////////////////////////////////////

int TRI_InsertKeySkipList (TRI_skiplist_t* skiplist,
                           TRI_skiplist_index_key_t* key,
                           TRI_skiplist_index_element_t* element,
                           bool overwrite);

////////////////////////////////////////////////////////////////////////////////
/// @brief lookups an element given a key, returns greatest left element
////////////////////////////////////////////////////////////////////////////////

TRI_skiplist_node_t* TRI_LeftLookupByKeySkipList (TRI_skiplist_t* skiplist,
                                                  TRI_skiplist_index_key_t* key);

////////////////////////////////////////////////////////////////////////////////
/// @brief lookups an element given a key, returns null if not found
////////////////////////////////////////////////////////////////////////////////

TRI_skiplist_node_t* TRI_LookupByKeySkipList (TRI_skiplist_t* skiplist,
                                              TRI_skiplist_index_key_t* key);

////////////////////////////////////////////////////////////////////////////////
/// @brief given a node returns the next node in the skip list, if the end is reached returns the end node
////////////////////////////////////////////////////////////////////////////////

TRI_skiplist_node_t* TRI_NextNodeSkipList (TRI_skiplist_t* skiplist,
                                           TRI_skiplist_node_t* currentNode);

////////////////////////////////////////////////////////////////////////////////
/// @brief given a node returns the prev node in the skip list, if the beginning is reached returns the start node
////////////////////////////////////////////////////////////////////////////////

TRI_skiplist_node_t* TRI_PrevNodeSkipList (TRI_skiplist_t* skiplist, 
                                           TRI_skiplist_node_t* currentNode);

////////////////////////////////////////////////////////////////////////////////
/// @brief removes an element from the skip list using element for comparison
////////////////////////////////////////////////////////////////////////////////

int TRI_RemoveElementSkipList (TRI_skiplist_t* skiplist,
                               TRI_skiplist_index_element_t* element, 
                               TRI_skiplist_index_element_t* old);

////////////////////////////////////////////////////////////////////////////////
/// @brief lookups an element given a key, returns least right element
////////////////////////////////////////////////////////////////////////////////

TRI_skiplist_node_t* TRI_RightLookupByKeySkipList (TRI_skiplist_t* skiplist,
                                                   TRI_skiplist_index_key_t* key);

////////////////////////////////////////////////////////////////////////////////
/// @brief returns the start node  which belongs to a skiplist
////////////////////////////////////////////////////////////////////////////////

void* TRI_StartNodeSkipList (TRI_skiplist_t*);

////////////////////////////////////////////////////////////////////////////////
/// @}
////////////////////////////////////////////////////////////////////////////////

// -----------------------------------------------------------------------------
// --SECTION--                                  non-unique skiplist public types
// -----------------------------------------------------------------------------

////////////////////////////////////////////////////////////////////////////////
/// @brief structure used for a multi skiplist
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
/// @addtogroup Skiplist_non_unique
/// @{
////////////////////////////////////////////////////////////////////////////////

typedef struct TRI_skiplist_multi_s {
  TRI_skiplist_base_t _base;

  // ...........................................................................
  // callback compare function
  // < 0: implies left < right
  // == 0: implies left == right
  // > 0: implies left > right
  // ...........................................................................

  int (*compareElementElement) (struct TRI_skiplist_multi_s*,
                                TRI_skiplist_index_element_t*,
                                TRI_skiplist_index_element_t*,
                                int);

  int (*compareKeyElement) (struct TRI_skiplist_multi_s*,
                            TRI_skiplist_index_key_t*,
                            TRI_skiplist_index_element_t*,
                            int);
  
  // ...........................................................................
  // Returns true if the element is an exact copy, or if the data which the
  // element points to is an exact copy
  // ...........................................................................

  bool (*equalElementElement) (struct TRI_skiplist_multi_s*,
                               TRI_skiplist_index_element_t*,
                               TRI_skiplist_index_element_t*);
}
TRI_skiplist_multi_t;

////////////////////////////////////////////////////////////////////////////////
/// @brief structure used for a multi skip list and is thread safe
////////////////////////////////////////////////////////////////////////////////

typedef struct TRI_skiplist_synced_multi_s {
  TRI_skiplist_t _base;
  TRI_read_write_lock_t _lock;  
} TRI_skiplist_synced_multi_t;

////////////////////////////////////////////////////////////////////////////////
/// @}
////////////////////////////////////////////////////////////////////////////////

// -----------------------------------------------------------------------------
// --SECTION--                 non-unique skiplist  constructors and destructors
// -----------------------------------------------------------------------------

////////////////////////////////////////////////////////////////////////////////
/// @addtogroup Skiplist_non_unique
/// @{
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
/// @brief initialises a skip list
////////////////////////////////////////////////////////////////////////////////

void TRI_InitSkipListMulti (TRI_skiplist_multi_t*,
                            TRI_skiplist_prob_e,
                            uint32_t);
                                              
////////////////////////////////////////////////////////////////////////////////
/// @brief destroys a multi skip list, but does not free the pointer
////////////////////////////////////////////////////////////////////////////////

void TRI_DestroySkipListMulti (TRI_skiplist_multi_t*);

////////////////////////////////////////////////////////////////////////////////
/// @brief destroys a skip list and frees the pointer
////////////////////////////////////////////////////////////////////////////////

void TRI_FreeSkipListMulti (TRI_skiplist_multi_t*);

////////////////////////////////////////////////////////////////////////////////
/// @}
////////////////////////////////////////////////////////////////////////////////

// -----------------------------------------------------------------------------
// --SECTION--                                 unique skiplist  public functions
// -----------------------------------------------------------------------------

////////////////////////////////////////////////////////////////////////////////
/// @addtogroup Skiplist_non_unique
/// @{
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
/// @brief returns the end node which belongs to a skiplist
////////////////////////////////////////////////////////////////////////////////

TRI_skiplist_node_t* TRI_EndNodeSkipListMulti (TRI_skiplist_multi_t* skiplist);

////////////////////////////////////////////////////////////////////////////////
/// @brief adds an element to the skip list using element for comparison
////////////////////////////////////////////////////////////////////////////////

int TRI_InsertElementSkipListMulti(TRI_skiplist_multi_t* skiplist,
                                   TRI_skiplist_index_element_t* element,
                                   bool overwrite);

////////////////////////////////////////////////////////////////////////////////
/// @brief lookups an element given a key, returns greatest left element
////////////////////////////////////////////////////////////////////////////////

TRI_skiplist_node_t* TRI_LeftLookupByKeySkipListMulti (TRI_skiplist_multi_t* skiplist,
                                                       TRI_skiplist_index_key_t* key);

////////////////////////////////////////////////////////////////////////////////
/// @brief given a node returns the next node in the skip list, if the end is reached returns the end node
////////////////////////////////////////////////////////////////////////////////

TRI_skiplist_node_t* TRI_NextNodeSkipListMulti (TRI_skiplist_multi_t* skiplist, 
                                                TRI_skiplist_node_t* currentNode);

////////////////////////////////////////////////////////////////////////////////
/// @brief given a node returns the prev node in the skip list, if the beginning is reached returns the start node
////////////////////////////////////////////////////////////////////////////////

TRI_skiplist_node_t* TRI_PrevNodeSkipListMulti (TRI_skiplist_multi_t* skiplist,
                                                TRI_skiplist_node_t* currentNode);

////////////////////////////////////////////////////////////////////////////////
/// @brief removes an element from the skip list using element for comparison
////////////////////////////////////////////////////////////////////////////////

int TRI_RemoveElementSkipListMulti (TRI_skiplist_multi_t* skiplist,
                                    TRI_skiplist_index_element_t* element,
                                    TRI_skiplist_index_element_t* old);

////////////////////////////////////////////////////////////////////////////////
/// @brief lookups an element given a key, returns least right element
////////////////////////////////////////////////////////////////////////////////

TRI_skiplist_node_t* TRI_RightLookupByKeySkipListMulti(TRI_skiplist_multi_t* skiplist,
                                                       TRI_skiplist_index_key_t* key);

////////////////////////////////////////////////////////////////////////////////
/// @brief returns the start node  which belongs to a skiplist
////////////////////////////////////////////////////////////////////////////////

TRI_skiplist_node_t* TRI_StartNodeSkipListMulti (TRI_skiplist_multi_t* skiplist);

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

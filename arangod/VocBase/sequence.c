////////////////////////////////////////////////////////////////////////////////
/// @brief sequences
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
/// @author Jan Steemann
/// @author Copyright 2013, triagens GmbH, Cologne, Germany
////////////////////////////////////////////////////////////////////////////////

#ifdef _WIN32
#include "BasicsC/win-utils.h"
#endif

#include "sequence.h"

// -----------------------------------------------------------------------------
// --SECTION--                                        constructors / destructors
// -----------------------------------------------------------------------------

////////////////////////////////////////////////////////////////////////////////
/// @addtogroup VocBase
/// @{
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
/// @brief create a new sequence
////////////////////////////////////////////////////////////////////////////////

TRI_sequence_t* TRI_CreateSequence (TRI_memory_zone_t* zone, 
                                    TRI_sequence_value_t initialValue) {
  TRI_sequence_t* sequence;

  sequence = TRI_Allocate(zone, sizeof(TRI_sequence_t), false);
  if (sequence != NULL) {
    TRI_InitSequence(sequence, initialValue);
  }

  return sequence;
}

////////////////////////////////////////////////////////////////////////////////
/// @brief initialise a sequence
////////////////////////////////////////////////////////////////////////////////

void TRI_InitSequence (TRI_sequence_t* sequence,
                       TRI_sequence_value_t initialValue) {
  TRI_InitSpin(&sequence->_lock);
  sequence->_value = initialValue;
}

////////////////////////////////////////////////////////////////////////////////
/// @brief destroy a sequence
////////////////////////////////////////////////////////////////////////////////

void TRI_DestroySequence (TRI_sequence_t* sequence) {
  assert(sequence != NULL);

  TRI_DestroySpin(&sequence->_lock);
}

////////////////////////////////////////////////////////////////////////////////
/// @brief destroy a sequence
////////////////////////////////////////////////////////////////////////////////

void TRI_FreeSequence (TRI_memory_zone_t* zone, 
                       TRI_sequence_t* sequence) {
  TRI_DestroySequence(sequence);
  TRI_Free(zone, sequence);
}

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
/// @brief atomically increase the value in the sequence
////////////////////////////////////////////////////////////////////////////////

TRI_sequence_value_t TRI_IncreaseSequence (TRI_sequence_t* sequence) {
  TRI_sequence_value_t value;

  TRI_LockSpin(&sequence->_lock);
  value = ++sequence->_value;
  TRI_UnlockSpin(&sequence->_lock);

  return value;
}

////////////////////////////////////////////////////////////////////////////////
/// @brief atomically get the value of the sequence 
////////////////////////////////////////////////////////////////////////////////

TRI_sequence_value_t TRI_GetValueSequence (TRI_sequence_t* sequence) {
  TRI_sequence_value_t value;

  TRI_LockSpin(&sequence->_lock);
  value = sequence->_value;
  TRI_UnlockSpin(&sequence->_lock);

  return value;
}

////////////////////////////////////////////////////////////////////////////////
/// @brief atomically set the value in the sequence to a new value
////////////////////////////////////////////////////////////////////////////////

void TRI_SetValueSequence (TRI_sequence_t* sequence, 
                           TRI_sequence_value_t value) {
  TRI_LockSpin(&sequence->_lock);

  if (value > sequence->_value) {
    sequence->_value = value;
  }

  TRI_UnlockSpin(&sequence->_lock);
}

////////////////////////////////////////////////////////////////////////////////
/// @brief set the value in the sequence, without acquiring the lock
////////////////////////////////////////////////////////////////////////////////

void TRI_SetValueNoLockSequence (TRI_sequence_t* sequence, 
                                 TRI_sequence_value_t value) {
  if (value > sequence->_value) {
    sequence->_value = value;
  }
}

////////////////////////////////////////////////////////////////////////////////
/// @}
////////////////////////////////////////////////////////////////////////////////

// Local Variables:
// mode: outline-minor
// outline-regexp: "^\\(/// @brief\\|/// {@inheritDoc}\\|/// @addtogroup\\|// --SECTION--\\|/// @\\}\\)"
// End:

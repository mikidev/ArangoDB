////////////////////////////////////////////////////////////////////////////////
/// @brief logger info
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
/// @author Achim Brandt
/// @author Copyright 2007-2013, triAGENS GmbH, Cologne, Germany
////////////////////////////////////////////////////////////////////////////////

#ifndef TRIAGENS_LOGGER_LOGGER_INFO_H
#define TRIAGENS_LOGGER_LOGGER_INFO_H 1

#include "Basics/Common.h"

#include "Logger/LoggerData.h"

namespace triagens {
  namespace basics {

// -----------------------------------------------------------------------------
// --SECTION--                                                  class LoggerInfo
// -----------------------------------------------------------------------------

////////////////////////////////////////////////////////////////////////////////
/// @addtogroup Logging
/// @{
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
/// @brief logger info
////////////////////////////////////////////////////////////////////////////////

    class LoggerInfo {
      friend class Logger;

////////////////////////////////////////////////////////////////////////////////
/// @}
////////////////////////////////////////////////////////////////////////////////

// -----------------------------------------------------------------------------
// --SECTION--                                      constructors and destructors
// -----------------------------------------------------------------------------

////////////////////////////////////////////////////////////////////////////////
/// @addtogroup Logging
/// @{
////////////////////////////////////////////////////////////////////////////////

      public:

////////////////////////////////////////////////////////////////////////////////
/// @brief constructs a logger info
////////////////////////////////////////////////////////////////////////////////

      LoggerInfo ();

////////////////////////////////////////////////////////////////////////////////
/// @brief destructs a logger info
////////////////////////////////////////////////////////////////////////////////

      virtual ~LoggerInfo ();

////////////////////////////////////////////////////////////////////////////////
/// @}
////////////////////////////////////////////////////////////////////////////////

// -----------------------------------------------------------------------------
// --SECTION--                                                    public methods
// -----------------------------------------------------------------------------

////////////////////////////////////////////////////////////////////////////////
/// @addtogroup Logging
/// @{
////////////////////////////////////////////////////////////////////////////////

      public:

////////////////////////////////////////////////////////////////////////////////
/// @brief catches a prefix
////////////////////////////////////////////////////////////////////////////////

        LoggerInfo& operator<< (string const& value);

////////////////////////////////////////////////////////////////////////////////
/// @brief catches a peg
////////////////////////////////////////////////////////////////////////////////

        LoggerInfo& operator<< (LoggerData::Peg const& value);

////////////////////////////////////////////////////////////////////////////////
/// @brief catches a task
////////////////////////////////////////////////////////////////////////////////

        LoggerInfo& operator<< (LoggerData::Task const& value);

////////////////////////////////////////////////////////////////////////////////
/// @brief catches an extra
////////////////////////////////////////////////////////////////////////////////

        LoggerInfo& operator<< (LoggerData::Extra const& value);

////////////////////////////////////////////////////////////////////////////////
/// @brief catches an user identifier
////////////////////////////////////////////////////////////////////////////////

        LoggerInfo& operator<< (LoggerData::UserIdentifier const& value);

////////////////////////////////////////////////////////////////////////////////
/// @brief catches a position
////////////////////////////////////////////////////////////////////////////////

        LoggerInfo& operator<< (LoggerData::Position const& value);

////////////////////////////////////////////////////////////////////////////////
/// @}
////////////////////////////////////////////////////////////////////////////////

// -----------------------------------------------------------------------------
// --SECTION--                                               protected variables
// -----------------------------------------------------------------------------

////////////////////////////////////////////////////////////////////////////////
/// @addtogroup Logging
/// @{
////////////////////////////////////////////////////////////////////////////////

      protected:

////////////////////////////////////////////////////////////////////////////////
/// @brief context
////////////////////////////////////////////////////////////////////////////////

        LoggerData::Info _info;
    };
  }
}

////////////////////////////////////////////////////////////////////////////////
/// @}
////////////////////////////////////////////////////////////////////////////////

#endif

// Local Variables:
// mode: outline-minor
// outline-regexp: "/// @brief\\|/// {@inheritDoc}\\|/// @addtogroup\\|/// @page\\|// --SECTION--\\|/// @\\}"
// End:

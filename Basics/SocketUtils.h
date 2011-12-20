////////////////////////////////////////////////////////////////////////////////
/// @brief collection of socket functions
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
/// @author Dr. Frank Celler
/// @author Copyright 2008-2011, triAGENS GmbH, Cologne, Germany
////////////////////////////////////////////////////////////////////////////////

#ifndef TRIAGENS_JUTLAND_BASICS_SOCKET_UTILS_H
#define TRIAGENS_JUTLAND_BASICS_SOCKET_UTILS_H 1

#include <BasicsC/Common.h>

namespace triagens {
  namespace basics {

    ////////////////////////////////////////////////////////////////////////////////
    /// @brief invalid socket
    ////////////////////////////////////////////////////////////////////////////////

    static int const INVALID_SOCKET = -1;

    ////////////////////////////////////////////////////////////////////////////////
    /// @brief collection of socket functions
    ////////////////////////////////////////////////////////////////////////////////

    namespace SocketUtils {

      ////////////////////////////////////////////////////////////////////////////////
      /// @brief thread save gethostbyname
      ////////////////////////////////////////////////////////////////////////////////

      char* gethostbyname (string const& hostname, size_t& length);

      ////////////////////////////////////////////////////////////////////////////////
      /// @brief sets non-blocking mode for a socket
      ////////////////////////////////////////////////////////////////////////////////

      bool setNonBlocking (socket_t fd);

      ////////////////////////////////////////////////////////////////////////////////
      /// @brief sets close-on-exit for a socket
      ////////////////////////////////////////////////////////////////////////////////

      bool setCloseOnExec (socket_t fd);
    }
  }
}


#endif

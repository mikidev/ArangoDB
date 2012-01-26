////////////////////////////////////////////////////////////////////////////////
/// @brief collections of file functions
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
/// @author Dr. Frank Celler
/// @author Copyright 2008-2012, triAGENS GmbH, Cologne, Germany
////////////////////////////////////////////////////////////////////////////////

#ifndef TRIAGENS_BASICS_FILE_UTILS_H
#define TRIAGENS_BASICS_FILE_UTILS_H 1

#include "Basics/Common.h"

namespace triagens {
  namespace basics {
    struct StringBuffer;

////////////////////////////////////////////////////////////////////////////////
/// @brief collection of file functions
////////////////////////////////////////////////////////////////////////////////

    namespace FileUtils {

////////////////////////////////////////////////////////////////////////////////
/// @brief returns a new ifstream or 0
////////////////////////////////////////////////////////////////////////////////

      ifstream * createInput (string const& filename);

////////////////////////////////////////////////////////////////////////////////
/// @brief returns a new ofstream or 0
////////////////////////////////////////////////////////////////////////////////

      ofstream * createOutput (string const& filename);

////////////////////////////////////////////////////////////////////////////////
/// @brief reads file into string
////////////////////////////////////////////////////////////////////////////////

      string slurp (string const& filename);

////////////////////////////////////////////////////////////////////////////////
/// @brief reads file into string buffer
////////////////////////////////////////////////////////////////////////////////

      void slurp (string const& filename, StringBuffer&);

////////////////////////////////////////////////////////////////////////////////
/// @brief creates file and writes string to it
////////////////////////////////////////////////////////////////////////////////

      void spit (string const& filename, string const& content);

////////////////////////////////////////////////////////////////////////////////
/// @brief creates file and writes string to it
////////////////////////////////////////////////////////////////////////////////

      void spit (string const& filename, StringBuffer const& content);

////////////////////////////////////////////////////////////////////////////////
/// @brief returns true if a file could be removed
////////////////////////////////////////////////////////////////////////////////

      bool remove (string const& fileName, int* errorNumber = 0);

////////////////////////////////////////////////////////////////////////////////
/// @brief returns true if a file could be renamed
////////////////////////////////////////////////////////////////////////////////

      bool rename (string const& oldName, string const& newName, int* errorNumber = 0);

////////////////////////////////////////////////////////////////////////////////
/// @brief creates a new directory
////////////////////////////////////////////////////////////////////////////////

      bool createDirectory (string const& name, int* errorNumber = 0);

////////////////////////////////////////////////////////////////////////////////
/// @brief creates a new directory with mask
////////////////////////////////////////////////////////////////////////////////

      bool createDirectory (string const& name, int mask, int* errorNumber = 0);

////////////////////////////////////////////////////////////////////////////////
/// @brief returns list of files
////////////////////////////////////////////////////////////////////////////////

      vector<string> listFiles (string const& directory);

////////////////////////////////////////////////////////////////////////////////
/// @brief checks if path is a directory
////////////////////////////////////////////////////////////////////////////////

      bool isDirectory (string const& path);

////////////////////////////////////////////////////////////////////////////////
/// @brief checks if path is a symbolic link
////////////////////////////////////////////////////////////////////////////////

      bool isSymbolicLink (string const& path);

////////////////////////////////////////////////////////////////////////////////
/// @brief checks if path is a regular file
////////////////////////////////////////////////////////////////////////////////

      bool isRegularFile (string const& path);

////////////////////////////////////////////////////////////////////////////////
/// @brief checks if path exists
////////////////////////////////////////////////////////////////////////////////

      bool exists (string const& path);

////////////////////////////////////////////////////////////////////////////////
/// @brief strip extension
////////////////////////////////////////////////////////////////////////////////

      string stripExtension (string const& path, string const& extension);

////////////////////////////////////////////////////////////////////////////////
/// @brief changes into directory
////////////////////////////////////////////////////////////////////////////////

      bool changeDirectory (string const& path);

////////////////////////////////////////////////////////////////////////////////
/// @brief returns the current directory
////////////////////////////////////////////////////////////////////////////////

      string currentDirectory (int* errorNumber);
    }
  }
}


#endif

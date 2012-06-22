////////////////////////////////////////////////////////////////////////////////
/// @brief plain http request
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
/// @author Dr. Frank Celler
/// @author Achim Brandt
/// @author Copyright 2004-2012, triAGENS GmbH, Cologne, Germany
////////////////////////////////////////////////////////////////////////////////

#ifndef TRIAGENS_REST_HTTP_REQUEST_PLAIN_H
#define TRIAGENS_REST_HTTP_REQUEST_PLAIN_H 1

#include "Rest/HttpRequest.h"

#include "Basics/Dictionary.h"

// -----------------------------------------------------------------------------
// --SECTION--                                            class HttpRequestPlain
// -----------------------------------------------------------------------------

////////////////////////////////////////////////////////////////////////////////
/// @addtogroup Rest
/// @{
////////////////////////////////////////////////////////////////////////////////

namespace triagens {
  namespace rest {

////////////////////////////////////////////////////////////////////////////////
/// @brief http request
///
/// The http server reads the request string from the client and converts it
/// into an instance of this class. An http request object provides methods to
/// inspect the header and parameter fields.
////////////////////////////////////////////////////////////////////////////////

    class  HttpRequestPlain : public HttpRequest {
      private:
        HttpRequestPlain (HttpRequestPlain const&);
        HttpRequestPlain& operator= (HttpRequestPlain const&);

////////////////////////////////////////////////////////////////////////////////
/// @}
////////////////////////////////////////////////////////////////////////////////

// -----------------------------------------------------------------------------
// --SECTION--                                      constructors and destructors
// -----------------------------------------------------------------------------

////////////////////////////////////////////////////////////////////////////////
/// @addtogroup Rest
/// @{
////////////////////////////////////////////////////////////////////////////////

      public:

////////////////////////////////////////////////////////////////////////////////
/// @brief http request constructor
///
/// Constructs a http request given the header string. A client request
/// consists of two parts: the header and the body. For a GET request the
/// body is always empty and all information about the request is delivered
/// in the header. For a POST or PUT request some information is also
/// delivered in the body. However, it is necessary to parse the header
/// information, before the body can be read.
////////////////////////////////////////////////////////////////////////////////

        HttpRequestPlain (char const* header, size_t length);

////////////////////////////////////////////////////////////////////////////////
/// @brief http request constructor
///
/// Constructs a http request given nothing. You can add the values,
/// the header information, and the path later.
////////////////////////////////////////////////////////////////////////////////

        HttpRequestPlain ();

////////////////////////////////////////////////////////////////////////////////
/// @brief destructor
////////////////////////////////////////////////////////////////////////////////

        ~HttpRequestPlain ();

////////////////////////////////////////////////////////////////////////////////
/// @}
////////////////////////////////////////////////////////////////////////////////

// -----------------------------------------------------------------------------
// --SECTION--                                               HttpRequest methods
// -----------------------------------------------------------------------------

////////////////////////////////////////////////////////////////////////////////
/// @addtogroup Rest
/// @{
////////////////////////////////////////////////////////////////////////////////

      public:

////////////////////////////////////////////////////////////////////////////////
/// {@inheritDoc}
////////////////////////////////////////////////////////////////////////////////

        char const* requestPath () const;

////////////////////////////////////////////////////////////////////////////////
/// {@inheritDoc}
////////////////////////////////////////////////////////////////////////////////

        void write (TRI_string_buffer_t*) const;

////////////////////////////////////////////////////////////////////////////////
/// {@inheritDoc}
////////////////////////////////////////////////////////////////////////////////

        size_t contentLength () const;

////////////////////////////////////////////////////////////////////////////////
/// {@inheritDoc}
////////////////////////////////////////////////////////////////////////////////

        char const* header (char const* field) const;

////////////////////////////////////////////////////////////////////////////////
/// {@inheritDoc}
////////////////////////////////////////////////////////////////////////////////

        char const* header (char const* field, bool& found) const;

////////////////////////////////////////////////////////////////////////////////
/// {@inheritDoc}
////////////////////////////////////////////////////////////////////////////////

        map<string, string> headers () const;

////////////////////////////////////////////////////////////////////////////////
/// {@inheritDoc}
////////////////////////////////////////////////////////////////////////////////

        char const* value (char const* key) const;

////////////////////////////////////////////////////////////////////////////////
/// {@inheritDoc}
////////////////////////////////////////////////////////////////////////////////

        char const* value (char const* key, bool& found) const;

////////////////////////////////////////////////////////////////////////////////
/// {@inheritDoc}
////////////////////////////////////////////////////////////////////////////////

        map<string, string> values () const;

////////////////////////////////////////////////////////////////////////////////
/// {@inheritDoc}
////////////////////////////////////////////////////////////////////////////////

        char const* body () const;

////////////////////////////////////////////////////////////////////////////////
/// {@inheritDoc}
////////////////////////////////////////////////////////////////////////////////

        size_t bodySize () const;

////////////////////////////////////////////////////////////////////////////////
/// {@inheritDoc}
////////////////////////////////////////////////////////////////////////////////

        int setBody (char const* newBody, size_t length);

////////////////////////////////////////////////////////////////////////////////
/// @}
////////////////////////////////////////////////////////////////////////////////

// -----------------------------------------------------------------------------
// --SECTION--                                                   private methods
// -----------------------------------------------------------------------------

////////////////////////////////////////////////////////////////////////////////
/// @addtogroup Rest
/// @{
////////////////////////////////////////////////////////////////////////////////

      private:

////////////////////////////////////////////////////////////////////////////////
/// @brief parses the http header
////////////////////////////////////////////////////////////////////////////////

        void parseHeader (char* ptr, size_t length);

////////////////////////////////////////////////////////////////////////////////
/// @brief sets the path of the request
///
/// @note The @FA{path} must exists as long as the instance is alive and it
///       must be garbage collected by the caller.
////////////////////////////////////////////////////////////////////////////////

        void setRequestPath (char const* path);

////////////////////////////////////////////////////////////////////////////////
/// @brief sets a key/value pair
////////////////////////////////////////////////////////////////////////////////

        void setValue (char const* key, char const* value);

////////////////////////////////////////////////////////////////////////////////
/// @brief sets the header values
////////////////////////////////////////////////////////////////////////////////

        void setValues (char* buffer, char* end);

////////////////////////////////////////////////////////////////////////////////
/// @brief set a header field
///
/// Set the value of a header field with given name.
////////////////////////////////////////////////////////////////////////////////

        void setHeader (char const* key, size_t keyLength, char const* value);

////////////////////////////////////////////////////////////////////////////////
/// @}
////////////////////////////////////////////////////////////////////////////////

// -----------------------------------------------------------------------------
// --SECTION--                                                 private variables
// -----------------------------------------------------------------------------

////////////////////////////////////////////////////////////////////////////////
/// @addtogroup Rest
/// @{
////////////////////////////////////////////////////////////////////////////////

      private:

////////////////////////////////////////////////////////////////////////////////
/// @brief complete request path, without protocol, host, and parameters
////////////////////////////////////////////////////////////////////////////////

        char const* _requestPath;

////////////////////////////////////////////////////////////////////////////////
/// @brief headers
////////////////////////////////////////////////////////////////////////////////

        basics::Dictionary<char const*> _headers;

////////////////////////////////////////////////////////////////////////////////
/// @brief values
////////////////////////////////////////////////////////////////////////////////

        basics::Dictionary<char const*> _values;

////////////////////////////////////////////////////////////////////////////////
/// @brief content length
////////////////////////////////////////////////////////////////////////////////

        size_t _contentLength;

////////////////////////////////////////////////////////////////////////////////
/// @brief body
////////////////////////////////////////////////////////////////////////////////

        char* _body;

////////////////////////////////////////////////////////////////////////////////
/// @brief body size
////////////////////////////////////////////////////////////////////////////////

        size_t _bodySize;

////////////////////////////////////////////////////////////////////////////////
/// @brief list of memory allocated which will be freed in the destructor
////////////////////////////////////////////////////////////////////////////////

        vector<char*> _freeables;
    };
  }
}

////////////////////////////////////////////////////////////////////////////////
/// @}
////////////////////////////////////////////////////////////////////////////////

#endif

// -----------------------------------------------------------------------------
// --SECTION--                                                       END-OF-FILE
// -----------------------------------------------------------------------------

// Local Variables:
// mode: outline-minor
// outline-regexp: "^\\(/// @brief\\|/// {@inheritDoc}\\|/// @addtogroup\\|/// @page\\|// --SECTION--\\|/// @\\}\\)"
// End:
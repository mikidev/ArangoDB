////////////////////////////////////////////////////////////////////////////////
/// @brief v8 client connection
///
/// @file
///
/// DISCLAIMER
///
/// Copyright by triAGENS GmbH - All rights reserved.
///
/// The Programs (which include both the software and documentation)
/// contain proprietary information of triAGENS GmbH; they are
/// provided under a license agreement containing restrictions on use and
/// disclosure and are also protected by copyright, patent and other
/// intellectual and industrial property laws. Reverse engineering,
/// disassembly or decompilation of the Programs, except to the extent
/// required to obtain interoperability with other independently created
/// software or as specified by law, is prohibited.
///
/// The Programs are not intended for use in any nuclear, aviation, mass
/// transit, medical, or other inherently dangerous applications. It shall
/// be the licensee's responsibility to take all appropriate fail-safe,
/// backup, redundancy, and other measures to ensure the safe use of such
/// applications if the Programs are used for such purposes, and triAGENS
/// GmbH disclaims liability for any damages caused by such use of
/// the Programs.
///
/// This software is the confidential and proprietary information of
/// triAGENS GmbH. You shall not disclose such confidential and
/// proprietary information and shall use it only in accordance with the
/// terms of the license agreement you entered into with triAGENS GmbH.
///
/// Copyright holder is triAGENS GmbH, Cologne, Germany
///
/// @author Dr. Frank Celler
/// @author Achim Brandt
/// @author Copyright 2008-2011, triagens GmbH, Cologne, Germany
////////////////////////////////////////////////////////////////////////////////

#include "V8ClientConnection.h"

#include <sstream>

#include "Basics/StringUtils.h"
#include "BasicsC/json.h"
#include "BasicsC/strings.h"
#include "Rest/HttpRequest.h"
#include "SimpleHttpClient/GeneralClientConnection.h"
#include "SimpleHttpClient/SimpleHttpClient.h"
#include "SimpleHttpClient/SimpleHttpResult.h"
#include "V8/v8-conv.h"

using namespace triagens::basics;
using namespace triagens::httpclient;
using namespace triagens::rest;
using namespace triagens::v8client;
using namespace std;

// -----------------------------------------------------------------------------
// --SECTION--                                      constructors and destructors
// -----------------------------------------------------------------------------

////////////////////////////////////////////////////////////////////////////////
/// @addtogroup V8ClientConnection
/// @{
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
/// @brief constructor
////////////////////////////////////////////////////////////////////////////////

V8ClientConnection::V8ClientConnection (Endpoint* endpoint,
                                        const string& username,
                                        const string& password,
                                        double requestTimeout,
                                        double connectTimeout,
                                        size_t numRetries,
                                        bool warn)
  : _connection(0),
    _lastHttpReturnCode(0),
    _lastErrorMessage(""),
    _client(0),
    _httpResult(0) {
  

  _connection = GeneralClientConnection::factory(endpoint, requestTimeout, connectTimeout, numRetries);

  if (_connection == 0) {
    throw "out of memory";
  }

  _client = new SimpleHttpClient(_connection, requestTimeout, warn);
  _client->setUserNamePassword("/", username, password);

  // connect to server and get version number
  map<string, string> headerFields;
  SimpleHttpResult* result = _client->request(HttpRequest::HTTP_REQUEST_GET, "/_api/version", 0, 0, headerFields);

  if (! result || ! result->isComplete()) {
    // save error message
    _lastErrorMessage = _client->getErrorMessage();
    _lastHttpReturnCode = 500;
  }
  else {
    _lastHttpReturnCode = result->getHttpReturnCode();

    if (result->getHttpReturnCode() == SimpleHttpResult::HTTP_STATUS_OK) {

      // default value
      _version = "arango";

      // convert response body to json
      TRI_json_t* json = TRI_JsonString(TRI_UNKNOWN_MEM_ZONE, result->getBody().str().c_str());

      if (json) {

        // look up "server" value (this returns a pointer, not a copy)
        TRI_json_t* server = TRI_LookupArrayJson(json, "server");

        if (server) {

          // "server" value is a string and content is "arango"
          if (server->_type == TRI_JSON_STRING && TRI_EqualString(server->_value._string.data, "arango")) {

            // look up "version" value (this returns a pointer, not a copy)
            TRI_json_t* vs = TRI_LookupArrayJson(json, "version");

            if (vs) {

              // "version" value is a string
              if (vs->_type == TRI_JSON_STRING) {
                _version = string(vs->_value._string.data, vs->_value._string.length - 1);
              }
            }
          }

          // must not free server and vs, they are contained in the "json" variable and freed below
        }

        TRI_FreeJson(TRI_UNKNOWN_MEM_ZONE, json);
      }
    }
    else {
      // initial request for /_api/version return some non-HTTP 200 response.
      // now set up an error message
      _lastErrorMessage = _client->getErrorMessage();
      if (result) {
        _lastErrorMessage = StringUtils::itoa(result->getHttpReturnCode()) + ": " + result->getHttpReturnMessage();
      }
    }
  }
 
  if (result) { 
    delete result; 
  }
}

////////////////////////////////////////////////////////////////////////////////
/// @brief destructor
////////////////////////////////////////////////////////////////////////////////

V8ClientConnection::~V8ClientConnection () {
  if (_httpResult) {
    delete _httpResult;
  }

  if (_client) {
    delete _client;
  }
  
  if (_connection) {
    delete _connection;
  }
}

////////////////////////////////////////////////////////////////////////////////
/// @}
////////////////////////////////////////////////////////////////////////////////

// -----------------------------------------------------------------------------
// --SECTION--                                                  public functions
// -----------------------------------------------------------------------------

////////////////////////////////////////////////////////////////////////////////
/// @addtogroup V8ClientConnection
/// @{
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
/// @brief returns true if it is connected
////////////////////////////////////////////////////////////////////////////////

bool V8ClientConnection::isConnected () {
  return _connection->isConnected();
}

////////////////////////////////////////////////////////////////////////////////
/// @brief returns the version and build number of the arango server
////////////////////////////////////////////////////////////////////////////////

const string& V8ClientConnection::getVersion () {
  return _version;
}

////////////////////////////////////////////////////////////////////////////////
/// @brief get the last http return code
////////////////////////////////////////////////////////////////////////////////

int V8ClientConnection::getLastHttpReturnCode () {
  return _lastHttpReturnCode;
}

////////////////////////////////////////////////////////////////////////////////
/// @brief get the last error message
////////////////////////////////////////////////////////////////////////////////

const std::string& V8ClientConnection::getErrorMessage () {
  return _lastErrorMessage;
}

////////////////////////////////////////////////////////////////////////////////
/// @brief get the simple http client
////////////////////////////////////////////////////////////////////////////////
            
triagens::httpclient::SimpleHttpClient* V8ClientConnection::getHttpClient() {
  return _client;
}

////////////////////////////////////////////////////////////////////////////////
/// @brief do a "GET" request
////////////////////////////////////////////////////////////////////////////////

v8::Handle<v8::Value> V8ClientConnection::getData (std::string const& location,
                                                   map<string, string> const& headerFields,
                                                   bool raw) {
  if (raw) {
    return requestDataRaw(HttpRequest::HTTP_REQUEST_GET, location, "", headerFields);
  }
  else {
    return requestData(HttpRequest::HTTP_REQUEST_GET, location, "", headerFields);
  }
}
    
////////////////////////////////////////////////////////////////////////////////
/// @brief do a "DELETE" request
////////////////////////////////////////////////////////////////////////////////

v8::Handle<v8::Value> V8ClientConnection::deleteData (std::string const& location,
                                                      map<string, string> const& headerFields,
                                                      bool raw) {
  if (raw) {
    return requestDataRaw(HttpRequest::HTTP_REQUEST_DELETE, location, "", headerFields);
  }
  else {
    return requestData(HttpRequest::HTTP_REQUEST_DELETE, location, "", headerFields);
  }
}
    
////////////////////////////////////////////////////////////////////////////////
/// @brief do a "HEAD" request
////////////////////////////////////////////////////////////////////////////////

v8::Handle<v8::Value> V8ClientConnection::headData (std::string const& location,
                                                    map<string, string> const& headerFields,
                                                    bool raw) {
  if (raw) {
    return requestDataRaw(HttpRequest::HTTP_REQUEST_HEAD, location, "", headerFields);
  }
  else {
    return requestData(HttpRequest::HTTP_REQUEST_HEAD, location, "", headerFields);
  }
}

////////////////////////////////////////////////////////////////////////////////
/// @brief do an "OPTIONS" request
////////////////////////////////////////////////////////////////////////////////

v8::Handle<v8::Value> V8ClientConnection::optionsData (std::string const& location,
                                                       std::string const& body,
                                                       map<string, string> const& headerFields,
                                                       bool raw) {
  if (raw) {
    return requestDataRaw(HttpRequest::HTTP_REQUEST_OPTIONS, location, body, headerFields);
  }
  else {
    return requestData(HttpRequest::HTTP_REQUEST_OPTIONS, location, body, headerFields);
  }
}

////////////////////////////////////////////////////////////////////////////////
/// @brief do a "POST" request
////////////////////////////////////////////////////////////////////////////////

v8::Handle<v8::Value> V8ClientConnection::postData (std::string const& location,
                                                    std::string const& body,
                                                    map<string, string> const& headerFields,
                                                    bool raw) {
  if (raw) {
    return requestDataRaw(HttpRequest::HTTP_REQUEST_POST, location, body, headerFields);
  }
  else {
    return requestData(HttpRequest::HTTP_REQUEST_POST, location, body, headerFields);
  }
}
    
////////////////////////////////////////////////////////////////////////////////
/// @brief do a "PUT" request
////////////////////////////////////////////////////////////////////////////////

v8::Handle<v8::Value> V8ClientConnection::putData (std::string const& location,
                                                   std::string const& body, 
                                                   map<string, string> const& headerFields,
                                                   bool raw) {
  if (raw) {
    return requestDataRaw(HttpRequest::HTTP_REQUEST_PUT, location, body, headerFields);
  }
  else {
    return requestData(HttpRequest::HTTP_REQUEST_PUT, location, body, headerFields);
  }
}

////////////////////////////////////////////////////////////////////////////////
/// @brief do a "PATCH" request
////////////////////////////////////////////////////////////////////////////////

v8::Handle<v8::Value> V8ClientConnection::patchData (std::string const& location,
                                                     std::string const& body, 
                                                     map<string, string> const& headerFields,
                                                     bool raw) {
  if (raw) {
    return requestDataRaw(HttpRequest::HTTP_REQUEST_PATCH, location, body, headerFields);
  }
  else {
    return requestData(HttpRequest::HTTP_REQUEST_PATCH, location, body, headerFields);
  }
}

////////////////////////////////////////////////////////////////////////////////
/// @}
////////////////////////////////////////////////////////////////////////////////

// -----------------------------------------------------------------------------
// --SECTION--                                                 private functions
// -----------------------------------------------------------------------------

////////////////////////////////////////////////////////////////////////////////
/// @addtogroup V8ClientConnection
/// @{
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
/// @brief executes a request
////////////////////////////////////////////////////////////////////////////////

v8::Handle<v8::Value> V8ClientConnection::requestData (HttpRequest::HttpRequestType method,
                                                       string const& location,
                                                       string const& body,
                                                       map<string, string> const& headerFields) {
  _lastErrorMessage = "";
  _lastHttpReturnCode = 0;
      
  if (_httpResult) {
    delete _httpResult;
  }
      
  if (body.empty()) {
    _httpResult = _client->request(method, location, 0, 0, headerFields);
  }
  else {
    _httpResult = _client->request(method, location, body.c_str(), body.length(), headerFields);
  }

  if (!_httpResult->isComplete()) {
    // not complete
    _lastErrorMessage = _client->getErrorMessage();
        
    if (_lastErrorMessage.empty()) {
      _lastErrorMessage = "Unknown error";
    }
        
    _lastHttpReturnCode = SimpleHttpResult::HTTP_STATUS_SERVER_ERROR;
        
    v8::Handle<v8::Object> result = v8::Object::New();
    result->Set(v8::String::New("error"), v8::Boolean::New(true));        
    result->Set(v8::String::New("code"), v8::Integer::New(SimpleHttpResult::HTTP_STATUS_SERVER_ERROR));
                
    int errorNumber = 0;

    switch (_httpResult->getResultType()) {
      case (SimpleHttpResult::COULD_NOT_CONNECT) :
        errorNumber = TRI_SIMPLE_CLIENT_COULD_NOT_CONNECT;
        break;
            
      case (SimpleHttpResult::READ_ERROR) :
        errorNumber = TRI_SIMPLE_CLIENT_COULD_NOT_READ;
        break;

      case (SimpleHttpResult::WRITE_ERROR) :
        errorNumber = TRI_SIMPLE_CLIENT_COULD_NOT_WRITE;
        break;

      default:
        errorNumber = TRI_SIMPLE_CLIENT_UNKNOWN_ERROR;
        break;
    }        
        
    result->Set(v8::String::New("errorNum"), v8::Integer::New(errorNumber));
    result->Set(v8::String::New("errorMessage"), v8::String::New(_lastErrorMessage.c_str(), _lastErrorMessage.length()));        

    return result;
  }
  else {
    // complete        
    _lastHttpReturnCode = _httpResult->getHttpReturnCode();
        
    // got a body
    if (_httpResult->getBody().str().length() > 0) {
      string contentType = _httpResult->getContentType(true);

      if (contentType == "application/json") {
        TRI_json_t* js = TRI_JsonString(TRI_UNKNOWN_MEM_ZONE, _httpResult->getBody().str().c_str());

        if (js != NULL) {
          // return v8 object
          v8::Handle<v8::Value> result = TRI_ObjectJson(js);
          TRI_FreeJson(TRI_UNKNOWN_MEM_ZONE, js);

          return result;
        }
      }

      // return body as string
      v8::Handle<v8::String> result = v8::String::New(_httpResult->getBody().str().c_str(), _httpResult->getBody().str().length());

      return result;
    }
    else {
      // no body 
      v8::HandleScope scope;

      v8::Handle<v8::Object> result = v8::Object::New();        
      
      result->Set(v8::String::New("code"), v8::Integer::New(_lastHttpReturnCode));

      if (_lastHttpReturnCode >= 400) {
        string returnMessage(_httpResult->getHttpReturnMessage());

        result->Set(v8::String::New("error"), v8::Boolean::New(true));
        result->Set(v8::String::New("errorNum"), v8::Integer::New(_lastHttpReturnCode));
        result->Set(v8::String::New("errorMessage"), v8::String::New(returnMessage.c_str(), returnMessage.size()));
      }
      else {
        result->Set(v8::String::New("error"), v8::Boolean::New(false));
      }

      return scope.Close(result);
    }        
  }      
}

////////////////////////////////////////////////////////////////////////////////
/// @brief executes a request and returns raw response
////////////////////////////////////////////////////////////////////////////////

v8::Handle<v8::Value> V8ClientConnection::requestDataRaw (HttpRequest::HttpRequestType method,
                                                          string const& location,
                                                          string const& body,
                                                          map<string, string> const& headerFields) {
  v8::HandleScope scope;

  _lastErrorMessage = "";
  _lastHttpReturnCode = 0;
      
  if (_httpResult) {
    delete _httpResult;
  }
      
  if (body.empty()) {
    _httpResult = _client->request(method, location, 0, 0, headerFields);
  }
  else {
    _httpResult = _client->request(method, location, body.c_str(), body.length(), headerFields);
  }

  if (!_httpResult->isComplete()) {
    // not complete
    _lastErrorMessage = _client->getErrorMessage();
        
    if (_lastErrorMessage.empty()) {
      _lastErrorMessage = "Unknown error";
    }
        
    _lastHttpReturnCode = SimpleHttpResult::HTTP_STATUS_SERVER_ERROR;
        
    v8::Handle<v8::Object> result = v8::Object::New();
    result->Set(v8::String::New("code"), v8::Integer::New(SimpleHttpResult::HTTP_STATUS_SERVER_ERROR));
                
    int errorNumber = 0;

    switch (_httpResult->getResultType()) {
      case (SimpleHttpResult::COULD_NOT_CONNECT) :
        errorNumber = TRI_SIMPLE_CLIENT_COULD_NOT_CONNECT;
        break;
            
      case (SimpleHttpResult::READ_ERROR) :
        errorNumber = TRI_SIMPLE_CLIENT_COULD_NOT_READ;
        break;

      case (SimpleHttpResult::WRITE_ERROR) :
        errorNumber = TRI_SIMPLE_CLIENT_COULD_NOT_WRITE;
        break;

      default:
        errorNumber = TRI_SIMPLE_CLIENT_UNKNOWN_ERROR;
        break;
    }        
        
    result->Set(v8::String::New("errorNum"), v8::Integer::New(errorNumber));
    result->Set(v8::String::New("errorMessage"), v8::String::New(_lastErrorMessage.c_str(), _lastErrorMessage.length()));        

    return scope.Close(result);
  }
  else {
    // complete        
    _lastHttpReturnCode = _httpResult->getHttpReturnCode();
        
    // create raw response
    v8::Handle<v8::Object> result = v8::Object::New();        

    result->Set(v8::String::New("code"), v8::Integer::New(_lastHttpReturnCode));

    if (_lastHttpReturnCode >= 400) {
      string returnMessage(_httpResult->getHttpReturnMessage());

      result->Set(v8::String::New("error"), v8::Boolean::New(true));
      result->Set(v8::String::New("errorNum"), v8::Integer::New(_lastHttpReturnCode));
      result->Set(v8::String::New("errorMessage"), v8::String::New(returnMessage.c_str(), returnMessage.size()));
    }
    else {
      result->Set(v8::String::New("error"), v8::Boolean::New(false));
    }

    // got a body, copy it into the result
    if (_httpResult->getBody().str().length() > 0) {
      v8::Handle<v8::String> body = v8::String::New(_httpResult->getBody().str().c_str(), _httpResult->getBody().str().length());

      result->Set(v8::String::New("body"), body);
    }

    // copy all headers
    v8::Handle<v8::Object> headers = v8::Object::New();
    const map<string, string>& headerFields = _httpResult->getHeaderFields();

    for (map<string, string>::const_iterator i = headerFields.begin();  i != headerFields.end();  ++i) {
      v8::Handle<v8::String> key = v8::String::New(i->first.c_str());
      v8::Handle<v8::String> val = v8::String::New(i->second.c_str());

      headers->Set(key, val);
    }

    result->Set(v8::String::New("headers"), headers);

    // and returns
    return scope.Close(result);
  }      
}

////////////////////////////////////////////////////////////////////////////////
/// @}
////////////////////////////////////////////////////////////////////////////////

// Local Variables:
// mode: outline-minor
// outline-regexp: "^\\(/// @brief\\|/// {@inheritDoc}\\|/// @addtogroup\\|// --SECTION--\\|/// @\\}\\)"
// End:

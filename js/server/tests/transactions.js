////////////////////////////////////////////////////////////////////////////////
/// @brief tests for transactions
///
/// @file
///
/// DISCLAIMER
///
/// Copyright 2010-2012 triagens GmbH, Cologne, Germany
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

var internal = require("internal");
var jsunity = require("jsunity");

// -----------------------------------------------------------------------------
// --SECTION--                                                        test suite
// -----------------------------------------------------------------------------

////////////////////////////////////////////////////////////////////////////////
/// @brief test suite
////////////////////////////////////////////////////////////////////////////////

function transactionsSuite () {

  return {

////////////////////////////////////////////////////////////////////////////////
/// @brief set up
////////////////////////////////////////////////////////////////////////////////

    setUp : function () {
    },

////////////////////////////////////////////////////////////////////////////////
/// @brief test: invalid invocations of TRANSACTION() function
////////////////////////////////////////////////////////////////////////////////

    testInvalidInvocations: function () {
      var tests = [
        undefined,
        null,
        true,
        false,
        0,
        1,
        "foo",
        { }, { },
        { }, { }, { },
        false, true,
        [ ],
        [ "action" ],
        [ "collections" ],
        [ "collections", "action" ],
        { },
        { collections: true },
        { action: true },
        { action: function () { } },
        { collections: true, action: true },
        { collections: { }, action: true },
        { collections: { } },
        { collections: true, action: function () { } },
        { collections: { read: true }, action: function () { } },
        { collections: { read: [ ], write: "foo" }, action: function () { } },
        { collections: { write: "foo" }, action: function () { } }
      ];

      tests.forEach(function (test) {
        try {
          TRANSACTION(test);
          fail();
        }
        catch (err) {
          assertEqual(internal.errors.ERROR_BAD_PARAMETER.code, err.errorNum);
        }
      });
    },

////////////////////////////////////////////////////////////////////////////////
/// @brief test: valid invocations of TRANSACTION() function
////////////////////////////////////////////////////////////////////////////////

    testValidEmptyInvocations: function () {
      var result;

      var tests = [
        { collections: { }, action: function () { result = 1; return true; } },
        { collections: { read: [ ] }, action: function () { result = 1; return true; } },
        { collections: { write: [ ] }, action: function () { result = 1; return true; } },
        { collections: { read: [ ], write: [ ] }, action: function () { result = 1; return true; } }
      ];

      tests.forEach(function (test) {
        result = 0;
          
        TRANSACTION(test);
        assertEqual(1, result);
      });
    }

  };
}

// -----------------------------------------------------------------------------
// --SECTION--                                                              main
// -----------------------------------------------------------------------------

////////////////////////////////////////////////////////////////////////////////
/// @brief executes the test suite
////////////////////////////////////////////////////////////////////////////////

jsunity.run(transactionsSuite);

return jsunity.done();

// -----------------------------------------------------------------------------
// --SECTION--                                                       END-OF-FILE
// -----------------------------------------------------------------------------

// Local Variables:
// mode: outline-minor
// outline-regexp: "\\(/// @brief\\|/// @addtogroup\\|// --SECTION--\\|/// @page\\|/// @\\}\\)"
// End:

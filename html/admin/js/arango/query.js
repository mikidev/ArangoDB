function initQuery () {
  $('#submitQueryButton').live('click', function () {
    submitQuery();
  });
  $('.clearicon').live('click', function () {
    var divname = $(this).next().attr('id');
    $('#'+divname).empty();
    $('#'+divname).val('');
  });
}

function submitQuery () {
  var data = {query:$('#queryInput').val()};
  var formattedJSON;

  $("#queryOutput").empty();
  $("#queryOutput").append('<pre class="preQuery">Loading...</pre>>'); 

  $.ajax({
    type: "POST",
    url: "/_api/cursor",
    data: JSON.stringify(data), 
    contentType: "application/json",
    processData: false, 
    success: function(data) {
      $("#queryOutput").empty();
      var formatQuestion = true;
      if (formatQuestion === true) {
        $("#queryOutput").append('<pre class="preQuery"><font color=green>' + FormatJSON(data.result) + '</font></pre>'); 
      }
      else {
        $("#queryOutput").append('<a class="querySuccess"><font color=green>' + JSON.stringify(data.result) + '</font></a>'); 
      }
    },
    error: function(data) {
      var temp = JSON.parse(data.responseText);
      $("#queryOutput").empty();
      $("#queryOutput").append('<a class="queryError"><font color=red>[' + temp.errorNum + '] ' + temp.errorMessage + '</font></a>'); 
    }
  });
}

function FormatJSON(oData, sIndent) {
    if (arguments.length < 2) {
        var sIndent = "";
    }
    var sIndentStyle = "    ";
    var sDataType = RealTypeOf(oData);

    if (sDataType == "array") {
        if (oData.length == 0) {
            return "[]";
        }
        var sHTML = "[";
    } else {
        var iCount = 0;
        $.each(oData, function() {
            iCount++;
            return;
        });
        if (iCount == 0) { // object is empty
            return "{}";
        }
        var sHTML = "{";
    }

    // loop through items
    var iCount = 0;
    $.each(oData, function(sKey, vValue) {
        if (iCount > 0) {
            sHTML += ",";
        }
        if (sDataType == "array") {
            sHTML += ("\n" + sIndent + sIndentStyle);
        } else {
            sHTML += ("\n" + sIndent + sIndentStyle + JSON.stringify(sKey) + ": ");
        }

        // display relevant data type
        switch (RealTypeOf(vValue)) {
            case "array":
            case "object":
                sHTML += FormatJSON(vValue, (sIndent + sIndentStyle));
                break;
            case "boolean":
            case "number":
                sHTML += vValue.toString();
                break;
            case "null":
                sHTML += "null";
                break;
            case "string":
                sHTML += "\"" + vValue.replace(/\\/g, "\\\\").replace(/"/g, "\\\"") + "\"";
                break;
            default:
                sHTML += ("TYPEOF: " + typeof(vValue));
        }

        // loop
        iCount++;
    });

    // close object
    if (sDataType == "array") {
        sHTML += ("\n" + sIndent + "]");
    } else {
        sHTML += ("\n" + sIndent + "}");
    }

    // return
    return sHTML;
}

function RealTypeOf(v) {
  if (typeof(v) == "object") {
    if (v === null) return "null";
    if (v.constructor == (new Array).constructor) return "array";
    if (v.constructor == (new Date).constructor) return "date";
    if (v.constructor == (new RegExp).constructor) return "regex";
    return "object";
  }
  return typeof(v);
}

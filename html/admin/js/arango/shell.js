var shArray = [];

///////////////////////////////////////////////////////////////////////////////
/// Initializing function for Shell View + Event-Binder for Buttons
///////////////////////////////////////////////////////////////////////////////

function initShell () {
  $("#avocshContent").autocomplete({
    source: shArray
  });
  $("#reloadShellButton").click(function() {
    reloadShell();
  });

  $("#submitShellButton").click(function() {
    submitShell();
  });

  $("#clearShellButton").click(function() {
    clearShell();
  });
  drawWelcomeMessage();

  $('.clearicon').live('click', function () {
    var divname = $(this).next().attr('id');
    $('#'+divname).empty();
    $('#'+divname).val('');
  });
  $('.reloadicon').live('click', function () {
    location.reload();
  });
  $('#shellInput').focus();


}

///////////////////////////////////////////////////////////////////////////////
/// Function for drawing the shell welcome message
///////////////////////////////////////////////////////////////////////////////

function drawWelcomeMessage () {
  if ($('#shellContent').text().length == 0 ) {
    $('#shellContent').append('<img src="pics/arangodblogo.png" style="width:141px; height:24px; margin-left:0;"></img>');
    print("Welcome to arangosh Copyright (c) 2012 triAGENS GmbH.");
    print(require("arangosh").HELP);
    start_pretty_print();
  }
}

///////////////////////////////////////////////////////////////////////////////
/// Function for executing the shell
///////////////////////////////////////////////////////////////////////////////

function submitShell () {
  var varInput = "#shellInput";
  var varContent = "#shellContent";
  var data = $(varInput).val();

  var r = [ ];
    for (var i = 0; i < shArray.length; ++i) {
      if (shArray[i] != data) {
        r.push(shArray[i]);
      }
    }

    shArray = r;
    if (shArray.length > 4) {
      shArray.shift();
    }
    shArray.push(data);

    $("#shellInput").autocomplete({
      source: shArray
    });

    if (data == "exit") {
      location.reload();
      return;
    }

    var command;
    if (data == "help") {
      command = "require(\"arangosh\").HELP";
    }
    else if (data == "reset") {
      command = "$('#shellContent').html(\"\");undefined;";
    }
    else {
      command = data;
    }

    var client = "arangosh> " + escapeHTML(data) + "<br>";
    $(varContent).append('<b class="shellClient">' + client + '</b>');
    evaloutput(command);
    $(varContent).animate({scrollTop:$(varContent)[0].scrollHeight}, 1);
    $(varInput).val('');
    return false;
}

///////////////////////////////////////////////////////////////////////////////
/// Evaluating Output + catching errors if available
///////////////////////////////////////////////////////////////////////////////

function evaloutput (data) {
  try {
    var result = eval(data); 
    if (result !== undefined) {
      print(result);
    }
  }
  catch(e) {
    $('#shellContent').append('<p class="shellError">Error:' + e + '</p>');
  }
}

///////////////////////////////////////////////////////////////////////////////
/// Clearing shell function
///////////////////////////////////////////////////////////////////////////////

function clearShell () {
  $('#shellContent').html("");
  console.log("clearing");
}

///////////////////////////////////////////////////////////////////////////////
/// Refresh shell
///////////////////////////////////////////////////////////////////////////////

function reloadShell () {
  location.reload();
}

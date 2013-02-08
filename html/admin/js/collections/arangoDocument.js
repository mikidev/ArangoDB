window.arangoDocument = Backbone.Collection.extend({
  url: '/_api/document/',
  model: arangoDocument,

  getDocument: function (colid, docid, view) {

    this.clearDocument();
    var self = this;
    $.ajax({
      type: "GET",
      url: "/_api/document/" + colid +"/"+ docid,
      contentType: "application/json",
      processData: false,
      success: function(data) {
        window.arangoDocumentStore.add(data);

        if (view == "source") {

        }
        else {
          window.documentView.initTable();
          window.documentView.drawTable();
        }
      },
      error: function(data) {
      }
    });
  },

  saveDocument: function () {

    var model = this.models[0].attributes;
    var docID = this.models[0].attributes._id;
    var hash = window.location.hash.split("/");

    $.ajax({
      type: "PUT",
      url: "/_api/document/" + docID,
      data: JSON.stringify(model),
      contentType: "application/json",
      processData: false,
      success: function(data) {
        console.log("saved");
      },
      error: function(data) {
        //alert(getErrorMessage(data));
        console.log(data);
      }
    });

  },

  updateLocalDocument: function (data) {
    this.clearDocument();
    this.add(data);
  },
  clearDocument: function () {
    window.arangoDocumentStore.reset();
  }

});

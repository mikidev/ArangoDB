window.arangoDocument = Backbone.Collection.extend({
  url: '/_api/document/',
  model: arangoDocument,

  getDocument: function (colid, docid) {
    var self = this;
    $.ajax({
      type: "GET",
      url: "/_api/document/" + colid +"/"+ docid,
      contentType: "application/json",
      processData: false,
      success: function(data) {
        window.arangoDocumentStore.add(data);
        window.documentView.initTable();
        window.documentView.drawTable();
      },
      error: function(data) {
      }
    });
  },
  saveDocument: function () {

  },
  deleteDocument: function () {

  },
  clearDocument: function () {
    window.arangoDocumentStore.reset();
  }

});

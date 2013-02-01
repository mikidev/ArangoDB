window.arangoDocument = Backbone.Model.extend({
  initialize: function () {
  },
  urlRoot: "/_api/document",
  defaults: {
    id: "",
    _rev: "",
  }
});

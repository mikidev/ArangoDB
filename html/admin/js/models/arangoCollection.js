window.arangoCollection = Backbone.Model.extend({
  url: '/_api/collection',
  initialize: function () {
  },
  defaults: {
    id: "",
    name: "",
    status: "",
    type: "",
    picture: ""
  }
});

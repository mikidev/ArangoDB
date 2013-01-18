window.arangoCollection = Backbone.Collection.extend({
      url: '/_api/collection',
      model: arangoCollection
});

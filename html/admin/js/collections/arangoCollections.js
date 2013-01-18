window.arangoCollections = Backbone.Collection.extend({
      url: '/_api/collection',
      parse: function(response)  {
          return response.collections;
      },
      model: arangoCollection
});

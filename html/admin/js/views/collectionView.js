var collectionView = Backbone.View.extend({
  el: '#content',
  init: function () {
    console.log(this.model.models);
  },

  template: new EJS({url: '/_admin/html/js/templates/collectionView.ejs'}),

  render: function() {
    return this;
  }

});

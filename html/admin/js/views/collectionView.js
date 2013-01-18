var collectionView = Backbone.View.extend({
  el: '#content',
  init: function () {
  },

  template: new EJS({url: '/_admin/html/js/templates/collectionView.ejs'}),

  render: function() {
    console.log(this.model.models);
    //$(this.el).html(this.template.text);
    //for (var i = 0; i < len; i++) {                                                                                                                                                            
    //  $('.thumbnails', this.el).append(new CollectionListItemView({model: collections[i]}).render().el);
    //}
    return this;
  }

});

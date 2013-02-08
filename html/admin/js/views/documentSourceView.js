var documentSourceView = Backbone.View.extend({
  el: '#content',
  init: function () {
  },
  events: {
    "click #tableView"   :   "tableView"
  },

  template: new EJS({url: '/_admin/html/js/templates/documentSourceView.ejs'}),

  render: function() {
    $(this.el).html(this.template.text);
    return this;
  },
  tableView: function () {
    console.log("ASDAS");
    var hash = window.location.hash.split("/");
    window.location.hash = hash[0]+"/"+hash[1]+"/"+hash[2];
  }

});

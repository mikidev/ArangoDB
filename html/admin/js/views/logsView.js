var logsView = Backbone.View.extend({
  el: '#content',
  offset: 0,
  size: 10,
  table: 'logTableID',

  initialize: function () {
  },

  events: {
    "click #all-switch" : "all",
    "click #error-switch" : "error",
    "click #warning-switch" : "warning",
    "click #debug-switch" : "debug",
    "click #info-switch" : "info"
  },
  all: function () {
    this.table = "logTableID";
    this.clearTable();
    this.collection.fillLocalStorage(this.table, 0, 10);
    this.drawTable();
  },
  error: function() {
    this.table = "critTableID";
    this.clearTable();
    this.collection.fillLocalStorage(this.table, 0, 10);
    this.drawTable();
  },
  warning: function() {
    this.table = "warnTableID";
    this.clearTable();
    this.collection.fillLocalStorage(this.table, 0, 10);
    this.drawTable();
  },
  debug: function() {
    this.table = "debugTableID";
    this.clearTable();
    this.collection.fillLocalStorage(this.table, 0, 10);
    this.drawTable();
  },
  info: function() {
    this.table = "infoTableID";
    this.clearTable();
    this.collection.fillLocalStorage(this.table, 0, 10);
    this.drawTable();
  },

  tabs: function () {
  },

  template: new EJS({url: '/_admin/html/js/templates/logsView.ejs'}),
  initLogTables: function () {
    var self = this;
    $.each(this.collection.tables, function(key, table) {
      table = $('#'+table).dataTable({
        "bFilter": false,
        "bPaginate": false,
        "bLengthChange": false,
        "bDeferRender": true,
        "bProcessing": true,
        "bAutoWidth": true,
        "iDisplayLength": -1,
        "bJQueryUI": false,
        "aoColumns": [{ "sClass":"center", "sWidth": "100px", "bSortable":false}, {"bSortable":false}],
        "oLanguage": {"sEmptyTable": "No logfiles available"}
      });
    });

  },
  render: function() {
    $(this.el).html(this.template.text);
    return this;
  },
  drawTable: function () {
    var self = this;
    $.each(this.collection.models, function(key, value) {
      $('#'+self.table).dataTable().fnAddData([value.attributes.level, value.attributes.text]);
    });
  },
  clearTable: function () {
    $('#'+this.table).dataTable().fnClearTable();
  }
});

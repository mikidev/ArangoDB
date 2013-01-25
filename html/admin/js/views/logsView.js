var logsView = Backbone.View.extend({
  el: '#content',
  offset: 0,
  size: 10,
  page: 1,
  table: 'logTableID',
  totalAmount: 0,
  totalPages: 0,

  initialize: function () {
    this.totalAmount = this.collection.models[0].attributes.totalAmount;
  },

  events: {
    "click #all-switch" : "all",
    "click #error-switch" : "error",
    "click #warning-switch" : "warning",
    "click #debug-switch" : "debug",
    "click #info-switch" : "info",
    "click #logTableID_first" : "firstTable",
    "click #logTableID_last"  : "lastTable",
    "click #logTableID_prev"  : "prevTable",
    "click #logTableID_next"  : "nextTable",
  },
  firstTable: function () {
    this.offset = 0;
    this.clearTable();
    this.collection.fillLocalStorage(this.table, this.offset, 10);
    this.drawTable();
  },
  lastTable: function () {
    this.totalPages = Math.ceil(this.totalAmount / 10);
    this.offset = (this.totalPages * 10) - 10;
    this.clearTable();
    this.collection.fillLocalStorage(this.table, this.offset, 10);
    this.drawTable();
  },
  prevTable: function () {
    this.offset = this.offset - this.size;
    this.clearTable();
    this.collection.fillLocalStorage(this.table, this.offset, 10);
    this.drawTable();
  },
  nextTable: function () {
    this.offset = this.offset + this.size;
    this.clearTable();
    this.collection.fillLocalStorage(this.table, this.offset, 10);
    this.drawTable();
  },
  all: function () {
    this.resetState();
    this.table = "logTableID";
    this.clearTable();
    this.collection.fillLocalStorage(this.table, this.offset, 10);
    this.drawTable();
  },
  error: function() {
    this.resetState();
    this.table = "critTableID";
    this.clearTable();
    this.collection.fillLocalStorage(this.table, this.offset, 10);
    this.drawTable();
  },
  warning: function() {
    this.resetState();
    this.table = "warnTableID";
    this.clearTable();
    this.collection.fillLocalStorage(this.table, this.offset, 10);
    this.drawTable();
  },
  debug: function() {
    this.resetState();
    this.table = "debugTableID";
    this.clearTable();
    this.collection.fillLocalStorage(this.table, 0, 10);
    this.drawTable();
  },
  info: function() {
    this.resetState();
    this.table = "infoTableID";
    this.clearTable();
    this.collection.fillLocalStorage(this.table, 0, 10);
    this.drawTable();
  },
  resetState: function () {
    this.offset = 0;
    this.size = 10;
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

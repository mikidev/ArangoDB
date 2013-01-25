$(document).ready(function() {

  window.Router = Backbone.Router.extend({

    routes: {
      ""                                    : "collections",
      "collection/:colid"                   : "collection",
      "collection/new"                      : "newCollection",
      "collection/:colid/documents/:pageid" : "documents",
      "collection/:colid/:docid"            : "document",
      "shell"                               : "shell",
      "dashboard"                           : "dashboard",
      "query"                               : "query",
      "logs"                                : "logs",
      "about"                               : "about"
    },
    initialize: function () {
      window.arangoCollectionsStore = new window.arangoCollections();
      window.arangoLogsStore = new window.arangoLogs();
      this.naviView = new window.navigationView();
      this.footerView = new window.footerView();
      this.naviView.render();
      this.footerView.render();
    },
    collections: function() {

      var naviView = this.naviView;

      window.arangoCollectionsStore.fetch({
        success: function () {
          var collectionsView = new window.collectionsView({
            collection: window.arangoCollectionsStore
          });
          collectionsView.render();
          naviView.selectMenuItem('collections-menu');
        }
      });
    },
    collection: function(colid) {
      //TODO: if-statement for every view needed!
       if (!this.collectionView) {
         this.collectionView = new window.collectionView({
           colId: colid,
           model: arangoCollection
         });
       }
       else {
         this.collectionView.options.colId = colid;
       }
       this.collectionView.render();
    },
    documents: function(colid) {
      this.documentsView = new window.documentsView();
      this.documentsView.render();
    },
    document: function(colid, docid) {
      this.documentView = new window.documentView();
      this.documentView.render();
    },
    shell: function() {
      this.shellView = new window.shellView();
      this.shellView.render();
      this.naviView.selectMenuItem('shell-menu');
    },
    query: function() {
      this.queryView = new window.queryView();
      this.queryView.render();
      this.naviView.selectMenuItem('query-menu');
    },
    about: function() {
      this.aboutView = new window.aboutView();
      this.aboutView.render();
      this.naviView.selectMenuItem('about-menu');
    },
    logs: function(loglevel) {
      if (typeof loglevel == 'undefined') {
        loglevel = "all";
      }
      var self = this;
      var temp = window.arangoLogsStore;
      window.arangoLogsStore.fetch({
        success: function () {
          this.logsView = new window.logsView({
            collection: temp
          });
          this.logsView.render();
          $('#logNav a[href="#'+loglevel+'"]').tab('show');
          this.logsView.initLogTables();
          this.logsView.drawTable("123");
        }
      });
      this.naviView.selectMenuItem('logs-menu');
    },
    dashboard: function() {
      this.dashboardView = new window.dashboardView();
      this.dashboardView.render();
      this.naviView.selectMenuItem('dashboard-menu');
    }

  });

  window.App = new window.Router();
  Backbone.history.start();

});

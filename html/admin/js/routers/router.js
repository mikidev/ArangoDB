$(document).ready(function() {

  window.Router = Backbone.Router.extend({

    routes: {
      ""                                    : "collections",
      "collection/:colid"                   : "collection",
      "new"                                 : "newCollection",
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

      window.arangoDocumentsStore = new window.arangoDocuments();
      window.documentsView = new window.documentsView({
        collection: window.arangoDocuments,
      });

      window.arangoLogsStore = new window.arangoLogs();
      window.arangoLogsStore.fetch({
        success: function () {
          if (!window.logsView) {
            console.log("not exits");
          }
          window.logsView = new window.logsView({
            collection: window.arangoLogsStore
          });
          //window.logsView.render();
          //$('#logNav a[href="#all"]').tab('show');
          //window.logsView.initLogTables();
          //window.logsView.drawTable();
        }
      });
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
            model: window.arangoCollectionsStore
          });
          collectionsView.render();
          naviView.selectMenuItem('collections-menu');
        }
      });
    },
    collection: function(colid) {
      //TODO: if-statement for every view !
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
    documents: function(colid, pageid) {

      window.arangoDocumentsStore.getDocuments(colid, pageid);
      if (!window.documentsView) {
        window.documentsView.initTable(colid, pageid);
      }
      window.documentsView.render();
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
    logs: function() {
      var self = this;
      window.arangoLogsStore.fetch({
        success: function () {
          if (!window.logsView) {
            console.log("not exits");
          }
          window.logsView.render();
          $('#logNav a[href="#all"]').tab('show');
          window.logsView.initLogTables();
          window.logsView.drawTable();
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

window.DocumentsView = Backbone.View.extend({

    initialize:function () {
    },
    initTable: function () {
      var documentsTable = $('#documentsTableID').dataTable({
        "bFilter": false,
        "bPaginate":false,
        "bSortable": false,
        "bLengthChange": false,
        "bDeferRender": true,
        "bAutoWidth": false,
        "iDisplayLength": -1,
        "aoColumns": [{ "sClass":"read_only leftCell", "bSortable": false, "sWidth":"80px"},
          { "sClass":"read_only","bSortable": false, "sWidth": "200px"},
          { "sClass":"read_only","bSortable": false, "sWidth": "100px"},
          { "sClass":"read_only","bSortable": false, "sWidth": "100px"},
          { "bSortable": false, "sClass": "cuttedContent rightCell"}],
          "oLanguage": { "sEmptyTable": "No documents"}
      });
    },
    fillTable: function () {
        collectioName
        currentPage
        var documentsPerPage = 10;
        var documentCount = 0;
        var totalPages;

        $.ajax({
          type: "GET",
          url: "/_api/collection/" + collectionName + "/count?" + getRandomToken(),
          contentType: "application/json",
          processData: false,
          async: false,
          success: function(data) {
            globalCollectionName = data.name;
            totalPages = Math.ceil(data.count / documentsPerPage);
            documentCount = data.count;
            $('#nav2').text(globalCollectionName);
          },
          error: function(data) {
          }
        });

        if (isNaN(currentPage) || currentPage == undefined || currentPage < 1) {
          currentPage = 1;
        }

        if (totalPages == 0) {
          totalPages = 1;
        }

        collectionCurrentPage = currentPage;
        collectionTotalPages = totalPages;
        var offset = (currentPage - 1) * documentsPerPage; // skip this number of documents
        $('#documentsTableID').dataTable().fnClearTable();

        $.ajax({
          type: 'PUT',
          url: '/_api/simple/all/',
          data: '{"collection":"' + collectionName + '","skip":' + offset + ',"limit":' + String(documentsPerPage) + '}',
          contentType: "application/json",
          success: function(data) {
            $.each(data.result, function(k, v) {
              $('#documentsTableID').dataTable().fnAddData(['<button class="enabled" id="deleteDoc"><img src="/_admin/html/media/icons/doc_delete_icon16.png" width="16" height="16"></button><button class="enabled" id="editDoc"><img src="/_admin/html/media/icons/doc_edit_icon16.png" width="16" height="16"></button>', v._id, v._key, v._rev, '<pre class=prettify>' + cutByResolution(JSON.stringify(v)) + '</pre>' ]);
            });
            $(".prettify").snippet("javascript", {style: "nedit", menu: false, startText: false, transparent: true, showNum: false});
            $('#documents_status').text(String(documentCount) + " document(s), showing page " + currentPage + " of " + totalPages);
          },
          error: function(data) {
          }
        });
    },
      render:function () {
        $(this.el).html(this.template());
        this.initTable();
        this.fillTable();
        return this;
    }

});

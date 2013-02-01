window.arangoDocuments = Backbone.Collection.extend({
      currentPage: 1,
      collectionID: 1,
      totalPages: 1,
      documentsPerPage: 10,
      documentsCount: 1,
      offset: 0,

      url: '/_api/documents',
      model: arangoDocument,

      getDocuments: function (colid, currpage) {
        var self = this;
        this.collectionID = colid;
        this.currentPage = currpage;

        $.ajax({
          type: "GET",
          url: "/_api/collection/" + this.collectionID + "/count",
          contentType: "application/json",
          processData: false,
          async: false,
          success: function(data) {
            self.totalPages = Math.ceil(data.count / this.documentsPerPage);
            self.documentsCount = data.count;
          },
          error: function(data) {
          }
        });


        if (isNaN(this.currentPage) || this.currentPage == undefined || this.currentPage < 1) {
          this.currentPage = 1;
        }

        if (this.totalPages == 0) {
          this.totalPages = 1;
        }

        this.offset = (this.currentPage - 1) * this.documentsPerPage;

        $.ajax({
          type: 'PUT',
          async: false,
          url: '/_api/simple/all/',
          data: '{"collection":"' + this.collectionID + '","skip":' + this.offset + ',"limit":' + String(this.documentsPerPage) + '}',
          contentType: "application/json",
          success: function(data) {
            self.clearDocuments();
            if (self.documentsCount != 0) {
              $.each(data.result, function(k, v) {
                window.arangoDocumentsStore.add({
                  "id": v._id,
                  "rev": v._rev,
                  "key": v._key,
                  "zipcode": v.zipcode,
                  "content": v
                });
                //$('#documentsTableID').dataTable().fnAddData(['<button class="enabled" id="deleteDoc"><img src="/_admin/html/media/icons/doc_delete_icon16.png" width="16" height="16"></button><button class="enabled" id="editDoc"><img src="/_admin/html/media/icons/doc_edit_icon16.png" width="16" height="16"></button>', v._id, v._key, v._rev, '<pre class=prettify>' + cutByResolution(JSON.stringify(v)) + '</pre>' ]);
              });
            }
            else {
              console.log("no data");
            }
            window.documentsView.drawTable();

            //$(".prettify").snippet("javascript", {style: "nedit", menu: false, startText: false, transparent: true, showNum: false});
            //$('#documents_status').text(String(documentCount) + " document(s), showing page " + currentPage + " of " + totalPages);
          },
          error: function(data) {
          }
        });
      },
      clearDocuments: function () {
        window.arangoDocumentsStore.reset();
      }
});

window.CollectionListView = Backbone.View.extend({

    initialize: function () {
        this.render();
        this.liveClick();
    },
    liveClick: function () {
        var self = this;
        var iconClass = '.icon-info-sign';
        $(iconClass).live('click', function () {
          self.fillModal(this.id);
        });
        $('#save-modified-collection').live('click', function () {
            self.updateCollection();
        });
    },
    updateCollection: function () {
      var checkCollectionName = $('#currentCollectionName').html();
      var self = this;

      //TODO: CHECK VALUES
      var wfscheck = $('#change-collection-sync').val();
      var journalSize = JSON.parse($('#change-collection-size').val() * 1024 * 1024);
      var wfs = (wfscheck == "true");
      var failed = false;
      var currentid = $('#change-collection-id').val();
      var newColName = $('#change-collection-name').val();

      console.log($('#change-collection-name').val());
      console.log("new coll name: "+ newColName);
      console.log("old coll name: "+ checkCollectionName);

      if (newColName != checkCollectionName) {
        $.ajax({
          type: "PUT",
          async: false, // sequential calls!
          url: "/_api/collection/" + checkCollectionName + "/rename",
          data: '{"name":"' + newColName + '"}',
          contentType: "application/json",
          processData: false,
          success: function(data) {
            arangoAlert("Collection renamed");
          },
          error: function(data) {
            console.log(JSON.stringify(data));
            //alert(getErrorMessage(data));
            failed = true;
          }
        });
      }
      if (! failed && window.store.collections[checkCollectionName] === 'loaded') {
        $.ajax({
          type: "PUT",
          async: false, // sequential calls!
          url: "/_api/collection/" + newColName + "/properties",
          data: '{"waitForSync":' + (wfs ? "true" : "false") + ',"journalSize":' + JSON.stringify(journalSize) + '}',
          contentType: "application/json",
          processData: false,
          success: function(data) {
            arangoAlert("Saved collection properties");
          },
          error: function(data) {
            //alert(getErrorMessage(data));
            console.log(JSON.stringify(data));
            failed = true;
          }
        });
      }
      if (! failed) {
        var tempCollection = window.store.collections[checkCollectionName];
        delete window.store.collections[checkCollectionName];
        window.store.collections[newColName] = tempCollection;
        window.store.collections[newColName].name = newColName;
        $('#change-collection').modal('hide')
        //TODO TODO TODO
/*        var collectionList = new CollectionCollection();
        collectionList.fetch({
          success: function() {
            $("#content").html(new CollectionListView({model: collectionList }).el);
          }
        });
        */
      }
      else {
        return 0;
      }
      window.location.hash = "";
      //this.liveClick();
    },
    deleteCollection: function () {
      //TODO: broken function
        var self = this;
        $.ajax({
          type: 'DELETE',
          url: "/_api/collection/" + self.model.attributes.id,
          success: function () {
            self.model.destroy({
              success: function () {
                window.history.back();
              }
            });
          },
          error: function () {
            alert('Error');
          }
        });
    },
    fillModal: function (collName) {
        $('#currentCollectionName').html(collName);
        console.log("hier : "+ collName);
        var tmpStore = window.store.collections[collName];
        console.log(tmpStore);
        $('#change-collection-name').val(collName);
        $('#change-collection-id').val(tmpStore.id);
        $('#change-collection-type').val(tmpStore.type);
        $('#change-collection-status').val(tmpStore.status);

        if (tmpStore.status == "unloaded") {
          $('#collectionSizeBox').hide();
          $('#collectionSyncBox').hide();
          $('#change-collection').modal('show')
        }
        else {
          $.ajax({
            type: "GET",
            url: "/_api/collection/" + collName + "/properties" + "?" + getRandomToken(),
            contentType: "application/json",
            processData: false,
            success: function(data) {
              $('#collectionSizeBox').show();
              $('#collectionSyncBox').show();
              if (data.waitForSync == false) {
                $('#change-collection-sync').val('false');
              }
              else {
                $('#change-collection-sync').val('true');
              }
              $('#change-collection-size').val(data.journalSize);
              $('#change-collection').modal('show')
            },
            error: function(data) {

            }
          });
        }
        return this;

    },
    render: function () {
        var self = this;

        var collections = this.model.models;
        var len = collections.length;
        //var startPos = (this.options.page - 1) * 20;
        //var endPos = Math.min(startPos + 20, len);
        $(this.el).html(this.template());

        //for (var i = startPos; i < endPos; i++) {
        for (var i = 0; i < len; i++) {
            $('.thumbnails', this.el).append(new CollectionListItemView({model: collections[i]}).render().el);
        }
        //$(this.el).append(new Paginator({model: this.model, page: this.options.page}).render().el);
          $('#save-new-collection').live('click', function () {
            self.saveNewCollection();
          });

        return this;
    },
    saveNewCollection: function () {
      var name = $('#new-collection-name').val();
      var size = $('#new-collection-size').val();
      var sync = $('#new-collection-sync').val();
      var type = $('#new-collection-type').val();
      var isSystem = (name.substr(0, 1) === '_');
      var journalSizeString = '';

      if (size == '') {
        journalSizeString = '';
      }
      else {
        size = JSON.parse(size) * 1024 * 1024;
        journalSizeString = ', "journalSize":' + size;
      }
      if (name == '') {
      }

      $.ajax({
        type: "POST",
        url: "/_api/collection",
        data: '{"name":' + JSON.stringify(name) + ',"waitForSync":' + JSON.parse(sync) + ',"isSystem":' + JSON.stringify(isSystem) + journalSizeString + ',"type":' + type + '}',
        contentType: "application/json",
        processData: false,
        success: function(data) {
          $.ajax({
            type: "GET",
            url: "/_api/collection/" + name,
            contentType: "application/json",
            processData: false,
            success: function(data) {
              var tmpStatus;
              tmpStatus = convertStatus(data.status);

              window.store.collections[data.name] = {
                "id":      data.id,
                "name":    data.name,
                "status":  tmpStatus,
                "type":    data.type,
                "picture": "database.gif"
              };
              arangoAlert("Collection created");
            },
            error: function(data) {
              arangoAlert("Collection error");
            }
          });
          $('#add-collection').modal('hide');
        },
        error: function(data) {
          return false;
        }
      });

    }
});

window.CollectionListItemView = Backbone.View.extend({

    tagName: "li",

    className: "span3",

    initialize: function () {
        this.model.bind("change", this.render, this);
        this.model.bind("destroy", this.close, this);
    },

    render: function () {
        $(this.el).html(this.template(this.model.toJSON()));
        return this;
    }

});

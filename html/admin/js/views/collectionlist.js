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
    },
    fillModal: function (collName) {
        if (window.store.collections[collName].status == "unloaded") {
          $('#collectionSizeBox').hide();
          $('#collectionSyncBox').hide();
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
              var tmpStatus = convertStatus(data.status);
              if (tmpStatus === "loaded") {
                $('#collectionBox').append('<a class="btn btn-unload pull-right collectionViewBtn" href="#">Unload</a>');
              }
            },
            error: function(data) {

            }
          });
        }
        return this;

    },
    render: function () {
        var funcVar = this;

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
            funcVar.saveNewCollection();
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

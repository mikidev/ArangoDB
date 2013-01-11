window.WineListView = Backbone.View.extend({

    initialize: function () {
        this.render();
    },

    render: function () {
        var funcVar = this;

        var collections = this.model.models;
        var len = collections.length;
        var startPos = (this.options.page - 1) * 20;
        var endPos = Math.min(startPos + 20, len);

        $(this.el).html('<div class="navcollection navbar-inner navbar-inner-custom">' +
                          '<a href="#add-collection" role="button" class="btn btn-primary" data-toggle="modal">Create</a>' +
                        '</div><ul class="thumbnails"></ul>');

        for (var i = startPos; i < endPos; i++) {
            $('.thumbnails', this.el).append(new CollectionListItemView({model: collections[i]}).render().el);
        }

        $(this.el).append(new Paginator({model: this.model, page: this.options.page}).render().el);

        $(this.el).append(''+
          '<div id="add-collection" class="modal hide fade" tabindex="-1" role="dialog" aria-labelledby="myModalLabel" aria-hidden="true">' +
            '<div class="modal-header">'+
              '<button type="button" class="close" data-dismiss="modal" aria-hidden="true">×</button>'+
              '<h3>New Collection</h3>'+
            '</div>'+
            '<div class="modal-body">'+
              '<table>'+
                '<tr>'+
                  '<th class="collectionTh">Name:</th>'+
                  '<th><input type="text" id="new-collection-name" name="name" value=""/></th>'+
                '</tr>'+
                '<tr>'+
                  '<th class="collectionTh">Size:</th>'+
                  '<th><input type="text" id="new-collection-size" name="size" value=""/></th>'+
                '</tr>'+
                '<tr>'+
                  '<th class="collectionTh">Sync:</th>'+
                  '<th>'+
                    '<select id="new-collection-sync">'+
                      '<option value="false">No</option>'+
                      '<option value="true">Yes</option>'+
                    '</select>'+
                  '</th>'+
                '</tr>'+
                '<tr>'+
                  '<th class="collectionTh">Type:</th>'+
                  '<th>'+
                    '<select id="new-collection-type">'+
                      '<option value="3">Document</option>'+
                      '<option value="2">Edge</option>'+
                    '</select>'+
                  '</th>'+
                '</tr>'+
              '</table>'+
            '</div>'+
            '<div class="modal-footer">'+
              '<button class="btn" data-dismiss="modal" aria-hidden="true">Abort</button>'+
              '<button id="save-new-collection" class="btn btn-primary">Create Collection</button>'+
            '</div>'+
          '</div>');

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
              switch (data.status) {
                case 1: tmpStatus = "new born collection"; break;
                case 2: tmpStatus = "unloaded"; break;
                case 3: tmpStatus = "loaded"; break;
                case 4: tmpStatus = "in the process of being unloaded"; break;
                case 5: tmpStatus = "deleted"; break;
              }

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

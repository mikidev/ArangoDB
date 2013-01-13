window.store = {
  collections: {},
  populate: function () {
    $.ajax({
      type: "GET",
      async: false,
      url: "/_api/collection",
      contentType: "application/json",
      success: function(data) {
        var tmpStatus;
        var tmpType;
        var i;
        for (i=0; i<data.collections.length; i++) {
          var collection = data.collections[i];
          if (collection.name.substr(0, 1) == "_") {
          }
          else {
            tmpStatus = convertStatus(collection.status);
            tmpType = convertType(collection.type);

            window.store.collections[collection.name] = {
              "id":      collection.id,
              "name":    collection.name,
              "status":  tmpStatus,
              "type":    tmpType,
              "picture": "database.gif"
            };
          }
        }
      }
    });
  },
  find: function (model) {
    return this.collections[model.id];
  },
  findAll: function () {
    return _.values(this.collections);
  },
  create: function (model) {
  },
  update: function (model) {
    return model;
  },
  notify: function () {
    console.log("changed");
  },

  destroy: function (model) {
    delete this.collections[model.attributes.name];
    arangoAlert("Collection deleted");
    return false;
  }

};

store.populate();
// Overriding Backbone's sync method. Replace the default RESTful services-based implementation
// with a simple in-memory approach.
Backbone.sync = function (method, model, options) {
    var resp;

    switch (method) {
        case "read":
            resp = model.id ? store.find(model) : store.findAll();
            break;
        case "create":
            resp = store.create(model);
            break;
        case "update":
            resp = store.update(model);
            break;
        case "delete":
            resp = store.destroy(model);
            break;
    }

    if (resp) {
        options.success(resp);
    } else {
        options.error("Record not found");
    }
};

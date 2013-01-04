// The in-memory Store. Encapsulates logic to access wine data.
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
        var i;
        for (i=0; i<data.collections.length; i++) {
          var collection = data.collections[i];

          switch (collection.status) {
            case 1: tmpStatus = "new born collection"; break; 
            case 2: tmpStatus = "unloaded"; break; 
            case 3: tmpStatus = "loaded"; break; 
            case 4: tmpStatus = "in the process of being unloaded"; break; 
            case 5: tmpStatus = "deleted"; break; 
          }

          window.store.collections[collection.name] = {
            "id":      collection.id,
            "name":    collection.name,
            "status":  tmpStatus,
            "type":    collection.type,
            "picture": "database.gif"
          };

        }
      //window.store.collections.lastId = data.collections.length;
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
    //this.collections[model.id] = model;
    return model;
  },

  destroy: function (model) {
    //delete this.collections[model.id];
    return model;
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

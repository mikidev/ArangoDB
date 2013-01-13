window.Collection = Backbone.Model.extend({

    initialize: function () {
        this.validators = {};

        this.validators.name = function (value) {
            return value.length > 0 ? {isValid: true} : {isValid: false, message: "You must enter a name"};
        };

    },

    defaults: {
        id: null,
        name: "",
        status: "",
        type: "",
        picture: null
    }
});

window.CollectionCollection = Backbone.Collection.extend({

    model: Collection

});

window.LogsView = Backbone.View.extend({

    initialize:function () {
        this.render();
        this.testing();
    },

    render:function () {
        $(this.el).html(this.template());
        return this;
    },
    testing:function () {
        $( "#logTabs" ).tabs();
     

    }
});

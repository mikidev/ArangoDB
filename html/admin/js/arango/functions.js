function countCollections () {
  var count = 0;
  self = store.collections;
  for(var prop in self) {
    if(self.hasOwnProperty(prop))
      ++count;
  }
  $('#transparentPlaceholder').html('<p class="pull-left white">Total collections: ' + count + '</p>');
  return count;
}

function hideArangoAlert() {
  $('.navlogo').tooltip('hide');
}

function arangoAlert(content) {
  $('.navlogo').tooltip('hide');
  $(".navlogo").data('tooltip', false).tooltip({
    delay: { show: 500, hide: 100 },
    'selector': '#1234567',
    'placement': 'bottom',
    'title': content,
  });

  $('.navlogo').tooltip('show');
  window.setTimeout(hideArangoAlert, 3000);

  return false;
}

function getRandomToken () {
  return Math.round(new Date().getTime());
}

function convertStatus(content) {
  var tmpStatus = "";
  switch (content) {
    case 1: tmpStatus = "new born collection"; break;
    case 2: tmpStatus = "unloaded"; break;
    case 3: tmpStatus = "loaded"; break;
    case 4: tmpStatus = "in the process of being unloaded"; break;
    case 5: tmpStatus = "deleted"; break;
  }
  return tmpStatus;
}

function convertType(content) {

}

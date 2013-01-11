

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

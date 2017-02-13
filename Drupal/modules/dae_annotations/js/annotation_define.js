$(document).ready(function(){
  var ui;
  
  $('#build-new-ui-builder').click(function(){
    var name = $('#annotation_name').val();
    var query = $('#user_query').val();
    var aid = $('#annotation_id').val();
    var yToScroll = $(this).offset().top;
    if(name && query){
      var vars = {'user_query' : query, 'annotation_name' : name};
      if(aid != '') {
        vars['annotation_id'] = aid;
      }
      $.post('?q=annotations/params', vars,
        function(data){
          buildUiBuilder(data, yToScroll);
        }, 'json');
    } else {
      alert('Please fill out both fields!');
    }
  });
  
  function buildUiBuilder(data, y){
    if (data.status == -1){
      alert('Error\n' + data.message);
      $('#annotation_name').val(ui.getTitle());
      $('#user_query').val(ui.getQuery());
    } else {
      ui = new DAE.AnnotationUI(data);
      $('body').animate({scrollTop : y+15});
    }
  }

});
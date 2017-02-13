<?php

function edit_annotation_info() {
  $aid = arg(3);
  if($aid && canEdit($aid)){
    drupal_add_js(drupal_get_path("module","dae_annotations") . '/js/annotation_define.js');
    drupal_add_css(drupal_get_path("module","dae_annotations") . '/stylesheets/annotations.css');
    $info = getAnnotationInfo($aid);
    return drupal_get_form('annotation_edit_info_form', $info);
  } else {
    drupal_goto('annotations');
  }
}

function annotation_edit_info_form($form_id, $info) {
  $form['annotation_name'] = array(
    '#type' => 'textfield',
    '#prefix' => '<br>Annotation name:',
    '#default_value' => $info['annotation_name'],
    '#required' => true
  );
  $form['user_query'] = array(
    '#type' => 'textarea',
    '#prefix' => 'Query to select data:',
    '#default_value' => $info['user_query'],
    '#required' => true
  );
  $form['annotation_id'] = array(
    '#type' => 'hidden',
    '#value' => $info['annotation_id'],
    '#required' => true
  );
  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => 'Save info'
  );
  return $form;
}

function annotation_edit_info_form_validate($form_id, &$form_state){
 //$check = validAnnotationInfo($name, $query);
 //if($check)
 //else form_set_error($check['message'])
}

function annotation_edit_info_form_submit($form_id, &$form_state){
  global $user;
  $uid = $user->uid;
  $data = array(
    'annotation_name' => $form_state['values']['annotation_name'],
    'edited' => (string)time(),
    'user_query' => $form_state['values']['user_query'],
    'annotation_id' => $form_state['values']['annotation_id'],
  );
  drupal_write_record('annotation_ui_storage', $data, 'annotation_id');
  drupal_set_message('Annotation ' .
    $form_state['values']['annotation_name'] .
    ' has successfully been updated.'
  );
  drupal_goto('annotations');
}


function edit_annotation_ui(){
  $aid = arg(3);
  if($aid && canEdit($aid)) {
    $params = dae_annotation_ui_params($aid);
    drupal_add_js(drupal_get_path("module","dae_annotations") . '/js/dae_annotations_builder.js');
    drupal_add_js('$(document).ready(function(){new DAE.AnnotationUI('.$params.')})', 'inline');
    drupal_add_css(drupal_get_path("module","dae_annotations") . '/stylesheets/annotations.css');
    return drupal_get_form('edit_annotation_ui_form', $aid);
  } else {
    drupal_goto('annotations');
  }
}

function edit_annotation_ui_form($fid, $aid){
  $form['console'] = array(
    '#type' => 'item',
    '#value' => '<div id="annotation-gui-console"></div>'
  );
  $form['xml'] = array(
    '#type' => 'textarea',
    '#prefix' => '<h3>Markup Specification</h3><p>Below is the XML specification that is used to construct your Annotation interface. It will automatically be updated as you build your interface.  Users can also input markup directly (<a href="#">See markup documentation</a>).',
    '#id' => 'annotation-markup-output',
  );
  $form['annotation_id'] = array(
    '#type' => 'hidden',
    '#value' => $aid
  );
  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => 'Save interface'
  );
  return $form;
}

function edit_annotation_ui_form_submit($form_id, &$form_state){
  global $user;
  $data = array(
    'edited' => (string)time(),
    'specification' => $form_state['values']['xml'],
    'annotation_id' => $form_state['values']['annotation_id']
  );
  drupal_write_record('annotation_ui_storage', $data, 'annotation_id');
  drupal_set_message('Annotation task has successfully been updated.'
  );
  drupal_goto('annotations');
}

?>
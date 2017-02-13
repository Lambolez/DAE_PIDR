<?php

function define_annotation_info(){
  return drupal_get_form('define_annotation_info_form');
}

function define_annotation_info_form(){
  $form['annotation_name'] = array(
    '#id' => 'annotation_name',
    '#prefix' => '<br>Annotation Name:',
    '#type' => 'textfield',
    '#required' => true
  );
  $form['user_query'] = array(
    '#prefix' => 'Query to select data:',
    '#id' => 'user_query',
    '#type' => 'textarea',
    '#required' => true
  );
  $form['proceed'] = array(
    '#type' => 'submit',
    '#value' => 'Build the interface'
  );
  $form['save'] = array(
    '#type' => 'submit',
    '#value' => 'Save and exit'
  );
  return $form;
}

function define_annotation_info_form_validate($fid, &$form){
  $name = $form['values']['annotation_name'];
  $query = $form['values']['user_query'];
  //if(!isset($name) || !isset($query)) {
    form_set_error($form['']);
  //}
}

function define_annotation_info_form_submit($fid, &$form_state) {
  global $user;
  $data = array(
    'annotation_name' => $form_state['values']['annotation_name'],
    'creator' => $user->uid,
    'created' => (string)time(),
    'edited' => (string)time(),
    'active' => TRUE,
    'user_query' => $form_state['values']['user_query']
  );
  $newAC = drupal_write_record('annotation_ui_storage', $data);
  if($form_state['clicked_button']['#value'] == $form_state['values']['proceed']){
    drupal_goto('annotations/edit/ui/' . $data['annotation_id']);
  } else {
    drupal_set_message('Annotation ' . $form_state['values']['annotation_name'] .
    ' has successfully been created.');
    drupal_goto('annotations');
  }
}

?>
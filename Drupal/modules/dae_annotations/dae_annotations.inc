<?php

function annotations(){
  drupal_add_css(drupal_get_path('module', 'dae_annotations') . '/stylesheets/annotations.css');
  drupal_set_title('Manage your annotation constraints');
  $acs = get_user_annotations();
  if($acs == null){
    $output = "You haven't created any annotation constraints yet.  <a href=\"?q=annotations/define/info\">Click here</a> to create one.";
  } else {
    $output = '<table>';
    $output .= '<tr><th>Name</th><th>Dataitems</th><th>Created</th><th>Edited</th><th colspan="3">Manage task</th></tr>';
    foreach($acs as $id => $info) {
      $output .= '<tr>';
      $output .= '<td><a href="?q=annotations/display/' . $id . '">' . $info['name'] . '</a></td>';
      $output .= '<td>' . $info['items'] . '</td>';
      $output .= '<td>' . format_date($info['created'], 'custom', 'H\:i \o\n M\. d\, Y') . '</td>';
      $output .= '<td>' . format_date($info['edited'], 'custom', 'H\:i \o\n M\. d\, Y') . '</td>';
      $output .= '<td><a href="?q=annotations/edit/info/' . $id . '">Edit Info</a></td>';
      $output .= '<td><a href="?q=annotations/edit/ui/' . $id . '">Edit UI</a></td>';
      $output .= '<td>' . 'Delete' . '</td>';
      $output .= '</tr>';
    }
  }
  $output .= '</table><a id="create-new-annotation-task" href="?q=annotations/define/info">'.
    '<span>+</span>Create a new task</a>';
  return $output;
}

function get_user_annotations(){
  global $user;
  $uid = $user->uid;
  $uis = db_query('
    SELECT annotation_name, annotation_id, created, edited, user_query
    FROM {annotation_ui_storage}
    WHERE creator = ' . $uid
  );
  while($row = db_fetch_array($uis)){
    $options[$row['annotation_id']] = array(
      'name' => $row['annotation_name'],
      'created' => $row['created'],
      'edited' => $row['edited'],
      'items' => annotation_sql_query_count($row['user_query'])
    );
  }
  return $options;
}

?>
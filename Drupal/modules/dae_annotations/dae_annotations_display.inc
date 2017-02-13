<?php

$ui;

function display_annotation() {
  $id = arg(2);
  
  $ui = db_fetch_array(db_query('
    SELECT annotation_name, specification, creator, user_query, count(*) as count
    FROM {annotation_ui_storage}
    WHERE annotation_id = ' . $id
  ));
  
  (arg(3)) ? $data = arg(3) : $data = 1;
  $dataitem = annotation_sql_query($ui['user_query'], $data);
  if(!$dataitem){
    drupal_set_message('No dataitem exists.  Please check the url.', 'error');
    return '';
  }
  
  drupal_set_title($ui['annotation_name']);
  $output = $data . ' of '. annotation_sql_query_count($ui['user_query']) . ' images';
  $xml = $ui['specification'];
  $ui_obj = new SimpleXMLElement($xml);
  
  $cropUI = new ImageCropUI("http://dae.cse.lehigh.edu/DAE/?q=browse/dataitem/thumb/" . $dataitem);
  $output .= drupal_get_form('user_gen_form', convertXmlObjToArr($ui_obj), $cropUI);
  $cropUI->build();
  
  return $output;
}


function get_widget_form_item($widget, $value){
  $type = (string) $widget['@attributes']['type'];
  $form_item = array();
  $form_item['#id'] = (string)$widget['@attributes']['name'];
  if($type == 'statictext'){
    $form_item['#type'] = 'item';
    $form_item['#value'] = '<strong>'.$widget['@text'].'</strong>';
  }
  
  else if($type == 'textarea') {
    $form_item['#prefix'] = '<strong>' . $widget['@text'] . '</strong>';
    $form_item['#type'] = 'textarea';
    if($value) {$form_item['#value'] = $value;}
  }
  
  else if($type == 'checkbox') {
    $options = array();
    foreach($widget['@children'] as $op) {
      $op = (string)$op['@text'];
      $options[$op] = $op;
    }
    $form_item['#type'] = 'checkboxes';
    $form_item['#options'] = $options;
    if($value) {$form_item['#default_value'] = $value;}
  }
  
  else if($type == 'radio') {
    $options = array();    
    foreach($widget['@children'] as $op) {
      $op = (string)$op['@text'];
      $options[$op] = $op;
    }
    $form_item['#type'] = 'radios';
    $form_item['#options'] = $options;
    if($value) {$form_item['#default_value'] = $value;}
  }
  
  return $form_item;
}




function user_gen_form($form_state, $ui, $cropUI){

  $form = array();
  
  //$form['#cache'] = TRUE;
  
  foreach($ui as $widget){
    if((string)$widget['@attributes']['type'] != 'markup') {
      if(isset($form_state['values'][(string)$widget['@attributes']['name']])){
        $value = $form_state['values'][(string)$widget['@attributes']['name']];
      } else {
        $value = false;
      }
      $form[(string)$widget['@attributes']['name']] =
        get_widget_form_item($widget, $value);
    } else {
      $form[(string)$widget['@attributes']['name']] = array(
        '#type' => 'item',
        '#value' => '<div id="image-crop-wrapper"></div>'
      );
      foreach($widget['@children'] as $tool){
        $cropUI->addTool((string)$tool['@attributes']['type']);
      }
    }
  }
  
  $form['tags'] = array(
    '#type' => 'hidden'
  );

  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => 'Submit'
  );
  
  return $form;
}

function user_gen_form_submit($id, &$form) {
  $output = json_decode($form['values']['tags']);
  drupal_set_message(json_encode($output));
}


function convertXmlObjToArr($obj) 
{
    $arr = array();
    $children = $obj->children(); 
    foreach ($children as $elementName => $node) 
    { 
        $nextIdx = count($arr); 
        $arr[$nextIdx] = array(); 
        $arr[$nextIdx]['@name'] = strtolower((string)$elementName); 
        $arr[$nextIdx]['@attributes'] = array(); 
        $attributes = $node->attributes(); 
        foreach ($attributes as $attributeName => $attributeValue) 
        { 
            $attribName = strtolower(trim((string)$attributeName)); 
            $attribVal = trim((string)$attributeValue); 
            $arr[$nextIdx]['@attributes'][$attribName] = $attribVal; 
        } 
        $text = (string)$node; 
        $text = trim($text); 
        if (strlen($text) > 0) 
        { 
            $arr[$nextIdx]['@text'] = $text; 
        } 
        $arr[$nextIdx]['@children'] = convertXmlObjToArr($node, $arr[$nextIdx]['@children']); 
    } 
    return $arr; 
}  

?>
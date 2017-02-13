<?php
/**
 * @file
 * 
 * @author  Mike Kot <mdk312@lehigh.edu>
 * 
 * @version 1.0
 *
 * 
 * @section LICENSE
 * 
 *  This file is part of the DAE Platform Project (DAE).
 *
 *  DAE is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  DAE is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with DAE.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

/**
 * This function contains and returns a tag form for data items
 *
 * @return - a drupal form array
 */
function dae_data_tag_form()
{

  $form['dae_data_tag'] = array(
    '#type' => 'textfield', 
  	'#required' => TRUE,
  );
   $form['dae_data_tag_on'] = array(
    '#type' => 'hidden', 
  	'#required' => TRUE,
  );
 $form['dae_data_tag_id'] = array(
    '#type' => 'hidden', 
  	'#required' => FALSE,
  );
  $form['dae_data_tag_submit'] = array(
    '#type' => 'submit',
    '#title' => t('Tag Dataitem'),
    '#default_value' => t('Tag'),
    '#description' => t('Submit'),
    '#suffix' => "<label id=\"dae-data-tag-on\">".t('Tag this page image.')."</label>"
  );

  
  return $form;
}
/**
 * This function validates user submitted tags by checking that they are more than one character
 * and they on a data item
 *  
 * @param $form - the drupal form object
 * @param $form_state - the user input
 *
 * @return - json encoded list of page elements and their boundaries relative to the fullsize
 *		 	 image.
 */
function dae_data_tag_form_validate($form, &$form_state)
{
	$form_state['values']['dae_data_tag'] = check_plain($form_state['values']['dae_data_tag']);
	
	if(strlen($form_state['values']['dae_data_tag']) < 1)
		form_set_error('', t('Tags must be more than 1 character.'));
	
	if(!is_numeric($form_state['values']['dae_data_tag_on']))
		form_set_error('', t('Unknown data item'));
	
	if(!empty($form_state['values']['dae_data_tag_id']) &&!is_numeric($form_state['values']['dae_data_tag_id']))
		form_set_error('', t('Bad Tag ID'));
}
/**
 * This function creates a new tag if necessary and then associates data items with the tag
 *  
 * @param $form_id - the drupal form id
 * @param $form_values - the user submitted form values
 *
 * @return - none
 */
function dae_data_tag_form_submit($form_id, $form_values)
{
	global $user;
	$db = getDAEDatabase();
	
	if($form_values['values']['dae_data_tag_id']) {
	    
	    settype($form_values['values']['dae_data_tag_id'], 'integer');
	    
		if($db->r("select ID from TAG where ID = ".$form_values['values']['dae_data_tag_id'],array()))
		{
			dae_data_tag_add_assoc($form_values['values']['dae_data_tag_on'],$form_values['values']['dae_data_tag_id']);
		}
		else
			form_set_error('', t('Bad Tag ID'));
	}
	else
	{
	    $variables[':tagvalue'] = $form_values['values']['dae_data_tag'];
		$exists = $db->r('select ID from TAG where VALUE =:tagvalue',$variables);
		
		if($exists)
			dae_data_tag_add_assoc($form_values['values']['dae_data_tag_on'],$exists);
		else
		{
			$new = $db->createUniqueEntry('TAG','ID');
			$db->i("update TAG set VALUE = :tagvalue where ID = $new",$variables);
			
			dae_data_tag_add_assoc($form_values['values']['dae_data_tag_on'],$new);
		
		}
	}
}
/**
 * This function is a helper for the tag form submit.  It associates tags and data items
 *  
 * @param $tag_on - the data item to associate a tag with
 * @param $tag_id - the id of the tag to associate with a data item
 *
 * @return - none
 */
function dae_data_tag_add_assoc($tag_on,$tag_id)
{
	$db = getDAEDatabase();
	
	settype($tag_on, 'integer');
	settype($tag_id, 'integer');
	
	$db->i("insert into ASSOCIATE_DATA_ITEM_TAG values($tag_on,$tag_id)",array());
	
}
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
 * This function contains the form for adding a comment to a data item
 *  
 * @param arg(2) is the id of the data item to comment on
 *
 * @return a drupal form array
 */
function dae_data_add_comment()
{
  
  $dataitem = arg(2);
  
  if(!check_dataset_id($dataitem))
  	return "";
  	

  $form['dae_data_comment'] = array(
    '#type' => 'textarea', 
  	'#required' => TRUE,
    '#default_value' => t('Comment on this data'),
    '#resizable' => FALSE
  );
  $form['dae_data_submit'] = array(
    '#type' => 'submit',
    '#title' => t('Submit'),
    '#default_value' => t('Add Comment'),
    '#description' => t('Submit')
  );
    $form['dae_data_item'] = array(
    '#type' => 'hidden',
    '#value' => t('!d',array('!d' => $dataitem)),
  );
  
  return $form;
}

/**
 * This function confirms that a comment is valid by checking to see that its body is larger than
 * 20 characters and that the author has rated the data item.
 *  
 * @param $form - drupal form object
 * @param $form_state - drupal form state object that contains user's input
 *
 * @return none
 */
function dae_data_add_comment_validate($form, &$form_state)
{
	if(strlen($form_state['values']['dae_data_comment']) < 20)
		 form_set_error('', t('Your comment must be more than 20 characters.'));
	
	if(!dae_data_rating_exists($form_state['values']['dae_data_item']))
		 form_set_error('', t('Please rate this dataset between 1 and 5 stars.'));
	
	$form_state['values']['dae_data_comment'] = check_plain($form_state['values']['dae_data_comment']);
	
}
/**
 * This function handles the submission of comments to the drupal database.
 *  
 * @param $form_id - drupal form identifier
 * @param $form_values - drupal form values array
 *
 * @return none
 */
function dae_data_add_comment_submit($form_id, $form_values)
{
	global $user;
	
	$date = strtotime(date('Y-m-d H:i:s'));
	
	db_query("INSERT into {dae_dataitem_comment} values(null,".$user->uid.",".$form_values['values']['dae_data_item'].",".$date.",'".$form_values['values']['dae_data_comment']."')");
	
}
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
 function dae_data_search_page() {
  $form = array();

  $form['dae_data_search'] = array(
    '#type' => 'textfield',
    '#attributes' => array('type' => 'search'),
    '#title' => t(''),
    '#default_value' => (!empty($_GET['s'])?$_GET['s']:'Search Names and Tags'),
    '#description' => t('')
  );


  return $form;
}

//added by yil308 for dropdown list.
 function dae_data_search_page_select() {
  $form = array();
  
  $db = getDAEDatabase();
  
						
	$importance = $db ->q("select tag.VALUE as VALUE, count(DATA_ITEM_ID) AS IMP 
						  from associate_data_item_tag data, TAG tag
						  where data.TAG_ID = tag.ID
						  group by tag.VALUE
						  order by count(DATA_ITEM_ID) desc, LOWER(tag.VALUE) asc ",array());
						
						
						
   $i = 1;
	$options = array();
	$options[0] = "- All tags -";
	while($row = $db->fetch_array($importance)):
	
		$options[$i] = $row[VALUE] . " (" . $row[IMP] . " items) ";
		$i++;
		
	endwhile;
	
	$form['dae_data_search'] = array(
    '#type' => 'textfield',
    '#attributes' => array('type' => 'search'),
    '#title' => t(''),
    '#default_value' => (!empty($_GET['s'])?$_GET['s']:'Search Names and Tags'),
    '#description' => t('')
  );
  
  $form['dae_data_search_dropdown'] = array(
    '#type' => 'select',
    '#attributes' => array('type' => 'select'),
    '#title' => t(''),
	'#default_value' => dae_data_search_page(),
    '#options' => $options,
    '#description' => t(''),
	'#id' => 'dae_data_search_dropdown',
  );

  return $form;
}

function dae_data_search_page_validate($form, &$form_state)
{
	$form_state['values']['dae_data_tag'] = check_plain($form_state['values']['dae_data_search']);
	$form_state['submitted'] = true;
}
function dae_data_search_page_submit($form_id, $form_values)
{
	global $user;
	$db = getDAEDatabase();
	
	$search = $form_values['values']['dae_data_tag'];
	
	drupal_goto("browse/search/results",array('s' => $search));
}


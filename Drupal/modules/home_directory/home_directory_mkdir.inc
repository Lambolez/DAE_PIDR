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
 
 
function home_directory_mkdir() 
{
  
  $loc = arg();
  unset($loc[0]);
  unset($loc[1]);

  $dir = implode("/",$loc);
  $dir .= "/";
  
  if(empty($dir))
  	$dir = '/';
  
  $form = array();
  
   $form['home_directory_dir'] = array(
    '#type' => 'textfield',
    '#title' => t('New Directory Name'),
    '#description' => t('Name of new subdirectory !dir without a trailing slash.',array('!dir' => 'of '.$dir))
  );
  $form['home_directory_path'] = array(
    '#type' => 'hidden',
    '#value' => t('!dir',array('!dir' => $dir)),
  );
  $form['home_directory_submit'] = array(
    '#type' => 'submit',
    '#title' => t('Submit'),
    '#default_value' => t('Create'),
    '#description' => t('Submit')
  );
  
  return $form;
}

/**
 * Implementation of mkdir_validate
 */
function home_directory_mkdir_validate($form, &$form_state) {

  if ($form_state['values']['home_directory_dir'] == 'o' || 
  	  $form_state['values']['home_directory_dir'] == 'commit') 
  	{
    form_set_error('', t('Invalid directory name.'));
  }
}
/**
 * Implementation of mkdir_submit.
 */
function home_directory_mkdir_submit($form_id, $form_values)
{
	global $uploads;
	
	if(!file_exists($uploads .'/'. $form_values['values']['home_directory_path']))
	{
		$form_values['values']['home_directory_path'] = "";
	}
	
	$dir = $form_values['values']['home_directory_dir'];
	$fullPathtoDir = $uploads .'/'. $form_values['values']['home_directory_path'];
	$relativePathtoDir = 'uploads/view/'.$form_values['values']['home_directory_path'];
	
	if(home_directory_make_directory($dir,$fullPathtoDir))
		drupal_goto($relativePathtoDir);
	else
		drupal_set_message(t('Directory %dir could not be created.', array('%dir' => $dir)),'error');
}
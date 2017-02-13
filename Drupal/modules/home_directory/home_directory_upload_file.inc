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
 * Implementation of upload.
 */
function home_directory_upload() {
  $form = array();
  $form['#attributes'] = array('enctype' => "multipart/form-data");
  
  $loc = arg();
  unset($loc[0]);
  
  foreach($loc as $folder):
	
			$dir.= $folder . "/";
	
  endforeach;
  
  $form['home_directory_file'] = array(
    '#type' => 'file',
    '#title' => t('File'),
    '#description' => t('File to upload')
  );
  $form['home_directory_path'] = array(
    '#type' => 'hidden',
    '#value' => t('!dir',array('!dir' => $dir)),
  );
  $form['home_directory_submit'] = array(
    '#type' => 'submit',
    '#title' => t('Submit'),
    '#default_value' => t('Upload'),
    '#description' => t('Submit')
  );
  return $form;
}

/**
 * Implementation of upload submit.
 */
function home_directory_upload_submit($form_id, $form_values)
{
 global $uploads, $user;

  $validators = array(
    'file_validate_extensions' => array(variable_get('home_directory_extensions', '')),
    'file_validate_size' => array(variable_get('home_directory_filesize', ''), variable_get('home_directory_size', '')),
  );
  // Save new file uploads.
 
 if(!file_exists($uploads .'/'. $form_values['values']['home_directory_path']))
	{
		$form_values['values']['home_directory_path'] = "";
	}

  $file = file_save_upload('home_directory_file', $validators);

  if($file)
  {  
	  $path = $uploads .'/'. $form_values['values']['home_directory_path'];
	
	  $filename=preg_replace("/[^a-zA-Z0-9\.\ ]/","_",$file->filename);  
	  $filename = home_directory_upload_rename($path,$filename);
	  
	  rename($file->destination, $path . $filename);
	  home_directory_change_usage(filesize($path. $filename));
	  drupal_goto('uploads/uploaded/' . $form_values['values']['home_directory_path'] . $filename);
  }
}

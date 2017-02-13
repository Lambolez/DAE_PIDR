<?php
// $Id:$

/**
 * @file
 * 
 * @author  Mike Kot <mdk312@lehigh.edu>
 * 
 * @version 1.0
 *
 * \brief Provides the administration page for home directory.
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
 * Administration settings page
 */ 
function home_directory_admin() {
  $form = array();
  $form['home_directory_userdata'] = array(
    '#type' => 'textfield',
    '#title' => t('Path to Userdata'),
    '#default_value' => variable_get('home_directory_userdata', '/dae/userdata/'),
    '#description' => t('Directory where user folders should be kept.  Default: ' . variable_get('home_directory_userdata', '/dae/userdata/'))
  );
  $form['home_directory_spool'] = array(
    '#type' => 'textfield',
    '#title' => t('Path to Algorithm Spool'),
    '#default_value' => variable_get('home_directory_spool', '/dae/spool/'),
    '#description' => t('Directory where algorithm input and output data is shared.  Default: ' . variable_get('home_directory_spool', '/dae/spool/'))
  );
  $form['home_directory_size'] = array(
    '#type' => 'textfield',
    '#title' => t('Storage Space'),
    '#default_value' => variable_get('home_directory_size', ''),
    '#description' => t('Maximum cumulative size of uploaded data in bytes.')
  );
  $form['home_directory_filesize'] = array(
    '#type' => 'textfield',
    '#title' => t('Max Upload Size'),
    '#default_value' => variable_get('home_directory_filesize', ''),
    '#description' => t('Maximum size of a single file upload in bytes.')
  );
  $form['home_directory_extensions'] = array(
    '#type' => 'textfield',
    '#title' => t('Allowed File extensions'),
    '#default_value' => variable_get('home_directory_extensions', ''),
    '#description' => t('File extensions allowed for upload separated by spaces.')
  );
  $form['home_directory_is_image'] = array(
    '#type' => 'textfield',
    '#title' => t('Image File Extensions'),
    '#default_value' => variable_get('home_directory_extensions', ''),
    '#description' => t('File extensions that can have thumbnails displayed separated by spaces.')
  );

  return system_settings_form($form);
}

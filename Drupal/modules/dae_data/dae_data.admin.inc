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
 * This function returns a drupal form array for the dae data settings
 *
 * @return - a settings form page
 */
 function dae_data_admin() {
  $form = array();

  $form['dae_data_is_image'] = array(
    '#type' => 'textfield',
    '#title' => t('Image File Extensions'),
    '#default_value' => variable_get('dae_data_extensions', 'png jpg jpeg tif tiff bmp gif'),
    '#description' => t('Filetypes that should display thumbnails.')
  );
  $form['dae_data_per_page'] = array(
    '#type' => 'textfield',
    '#title' => t('Data items per page'),
    '#default_value' => variable_get('dae_data_per_page', '10'),
    '#description' => t('The amount of data items to display to users per page.')
  );
  $form['dae_data_popular_interval'] = array(
    '#type' => 'select',
    '#title' => t('Default interval for browsing popular data items'),
    '#default_value' => variable_get('dae_data_popular_interval','month'),
    '#description' => t('The amount time to consider when looking at popular data items.'),
    '#options' => array(
     1 => t('Today'), 
     7 => t('This Week'), 
     30 => t('This Month'),
     365 => t('This Year')
  	),
  );
  $form['dae_data_size_compute'] = array(
    '#type' => 'checkbox',
    '#title' => t('Compute Data Download Size'),
    '#default_value' => variable_get('dae_data_size_compute',false),
    '#description' => t('Select whether data dowload size is computed and displayed (may cause significant overhead and may not work on standalone versions without actual access to data)'),
  );
  $form['dae_data_passthrough_ip'] = array(
    '#type' => 'textfield',
    '#title' => t('Passthrough IP addresses'),
    '#default_value' => variable_get('dae_data_passthrough_ip', ''),
    '#description' => t('IP addressess that get unconstrained data downloading (no authentication required).')
  );
  $form['dae_data_preparse_script'] = array(
    '#type' => 'textfield',
    '#title' => t('Upload Preparation Script'),
    '#default_value' => variable_get('dae_data_preparse_script', ''),
    '#description' => t('Script that is used to prepare upload of dataset archive.')
  );
  $form['dae_data_upload_dir'] = array(
    '#type' => 'textfield',
    '#title' => t('Temporary Upload Directory'),
    '#default_value' => variable_get('dae_data_upload_dir', '/dae/upload'),
    '#description' => t('Directory used to temporarily store dataset archives for uploading.')
  );
  $form['dae_data_upload_script'] = array(
    '#type' => 'textfield',
    '#title' => t('Upload Script'),
    '#default_value' => variable_get('dae_data_upload_script', ''),
    '#description' => t('Script that is used to upload a dataset archive.')
  );
  return system_settings_form($form);
}
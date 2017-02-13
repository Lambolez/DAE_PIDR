<?php
// $Id:$

/**
 * @file
 * Provides the administration page for home directory.
 */
  
/**
 * Administration settings page
 */ 
function home_directory_admin() {
  $form = array();
  $form['home_directory_userdata'] = array(
    '#type' => 'textfield',
    '#title' => t('Path to Userdata'),
    '#default_value' => variable_get('home_directory_userdata', '/dae/userdata'),
    '#description' => t('Directory where user folders should be kept.  Default: ' . variable_get('home_directory_userdata', '/dae/userdata'))
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

  return system_settings_form($form);
}

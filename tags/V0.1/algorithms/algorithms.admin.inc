<?php
// $Id:$

/**
 * @file
 * Provides the administration page for algorithms.
 */
  
/**
 * Administration settings page
 */ 
function algorithms_admin() {
  $form = array();
  $form['algorithms_app_server'] = array(
    '#type' => 'textfield',
    '#title' => t('Server Hostname'),
    '#default_value' => variable_get('algorithms_app_server', ''),
    '#description' => t('This is the hostname of the server to ssh into when calling an algorithm')
  );  
  $form['algorithms_ssh_id_file'] = array(
    '#type' => 'textfield',
    '#title' => t('SSH Identity File'),
    '#default_value' => variable_get('algorithms_ssh_id_file', '~/.ssh/id_rsa'),
    '#description' => t('This is the filename containing the SSH key for remote execution')
  );
  $form['algorithms_concurrent_runs'] = array(
    '#type' => 'textfield',
    '#title' => t('Maximum Concurrent Runs'),
    '#default_value' => variable_get('algorithms_concurrent_runs', ''),
    '#description' => t('This is maximum number of runs a user may initiate at one time.  Users given unlimited runs are unrestricted.')
  );

  return system_settings_form($form);
}

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
	
	/**
	 * \todo These values should be global variables
	 */
	$processUser = posix_getpwuid(posix_geteuid());
	$processUsername = $processUser['name'];
	
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
   $form['algorithms_ssh_username'] = array(
    '#type' => 'textfield',
    '#title' => t('SSH User Identity'),
    '#default_value' => variable_get('algorithms_ssh_username', $processUsername),
    '#description' => t('This is the username that is used for SSH remote execution')
  );
  $form['algorithms_concurrent_runs'] = array(
    '#type' => 'textfield',
    '#title' => t('Maximum Concurrent Runs'),
    '#default_value' => variable_get('algorithms_concurrent_runs', ''),
    '#description' => t('This is maximum number of runs a user may initiate at one time.  Users given unlimited runs are unrestricted.')
  );
	$form['algorithms_convert_output'] = array(
    '#type' => 'textfield',
    '#title' => t('Allowed Output Types Using Convert'),
    '#default_value' => variable_get('algorithms_convert_output', ''),
    '#description' => t('Enter the output types allowed when using convert separated by spaces.')
  );

  return system_settings_form($form);
}

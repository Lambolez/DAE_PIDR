<?php
// $Id:$

/**
 * @file
 * Provides the administration page for algorithms.
 */
  
/**
 * Administration settings page
 */ 
function daedatabase_admin() {
  $form = array();
  $form['dae_db_server'] = array(
    '#type' => 'textfield',
    '#title' => t('Database Server Hostname'),
    '#default_value' => variable_get('dae_db_server', 'dae-oracle.cse.lehigh.edu:1521'),
    '#description' => t('This is the hostname of the server to ssh into when calling an algorithm')
  );
  $form['dae_db_application'] = array(
    '#type' => 'radios',
    '#title' => t('Database Management System'),
    '#default_value' => variable_get('dae_db_application', 0),
    '#options' => array(t('Oracle'), t('MySQL local')),
    '#description' => t('Decides whether data is retrieved from off-line Oracle or local MySQL database')
  );
  $form['dae_db_user'] = array(
    '#type' => 'textfield',
    '#title' => t('Database User Name'),
    '#default_value' => variable_get('dae_db_user', 'webserver'),
    '#description' => t('User name to connect to database')
  );
  $form['dae_db_pass'] = array(
    '#type' => 'textfield',
    '#title' => t('Database Password'),
    '#default_value' => variable_get('dae_db_pass', 'webserver$dae$0$'),
    '#description' => t('Password to connect to database')
  );
  $form['dae_db_name'] = array(
    '#type' => 'textfield',
    '#title' => t('Database Name/SID'),
    '#default_value' => variable_get('dae_db_name', 'dae0'),
    '#description' => t('Decides name or SID to use')
  );

  return system_settings_form($form);
}


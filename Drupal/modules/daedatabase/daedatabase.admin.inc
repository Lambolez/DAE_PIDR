<?php
// $Id:$

/**
 * @file daedatabase.admin.inc
 * @author  Bart Lamiroy <lamiroy@cse.lehigh.edu>
 * @version 1.0
 *
 * \brief Provides the administration page for module daedatabase.
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
 * \brief This is a standard Drupal hook for Module administration.
 *
 * Module settings page
 * 
 */ 
function daedatabase_admin() {
  $form = array();
  //!  - \a dae_db_server - default value 'dae-oracle.cse.lehigh.edu'. 
  $form['dae_db_server'] = array(
    '#type' => 'textfield',
    '#title' => t('Database Server Hostname'),
    '#default_value' => variable_get('dae_db_server', 'dae-oracle.cse.lehigh.edu'), 
    '#description' => t('This is the hostname of the server to ssh into when calling an algorithm')
  );
  //!  - \a dae_db_application - default value 'Oracle'. Decides whether data is retrieved from on-line Oracle or local MySQL database
  $form['dae_db_application'] = array(
    '#type' => 'radios',
    '#title' => t('Database Management System'),
    '#default_value' => variable_get('dae_db_application', 0),
    '#options' => array(t('Oracle'), t('MySQL local')),
    '#description' => t('Decides whether data is retrieved from off-line Oracle or local MySQL database')
  );
  //!  - \a dae_db_user - User name to connect to database
  $form['dae_db_user'] = array(
    '#type' => 'textfield',
    '#title' => t('Database User Name'),
    '#default_value' => variable_get('dae_db_user', ''),
    '#description' => t('User name to connect to database')
  );
  //!  - \a dae_db_pass - Database Password for \a dae_db_user
  $form['dae_db_pass'] = array(
    '#type' => 'textfield',
    '#title' => t('Database Password'),
    '#default_value' => variable_get('dae_db_pass', ''),
    '#description' => t('Password to connect to database')
  );
  //!  - \a dae_db_user - User name to connect to database
  $form['dae_db_readonly_user'] = array(
    '#type' => 'textfield',
    '#title' => t('Read Only Database User Name'),
    '#default_value' => variable_get('dae_db_readonly_user', ''),
    '#description' => t('User name to connect to database in restricted access mode')
  );
  //!  - \a dae_db_pass - Database Password for \a dae_db_user
  $form['dae_db_readonly_pass'] = array(
    '#type' => 'textfield',
    '#title' => t('Read Only Database Password'),
    '#default_value' => variable_get('dae_db_readonly_pass', ''),
    '#description' => t('Password to connect to database in restricted access mode')
  );
  //!  - \a dae_db_name - default value 'daetoy'. Decides database name or SID to use
  $form['dae_db_name'] = array(
    '#type' => 'textfield',
    '#title' => t('Database Name/SID'),
    '#default_value' => variable_get('dae_db_name', ''),
    '#description' => t('Decides database name or SID to use')
  );

  return system_settings_form($form);
}


<?php
// $Id:$

/**
 * @file 
 * 
 * @author  Mike Kot <mdk312@lehigh.edu>
 * 
 * @version 1.0
 *
 * \brief Provides the administration page for algorithms.
 *  SELECT DISTINCT alg.name, dit_in.id

            FROM algorithm_run arun   

           join algorithm_run_of arof on
           arof.algorithm_run_id = arun.id

           join algorithm alg on
           alg.id = arof.algorithm_id
           
            join algorithm_run_intput arin on
           arin.algorithm_run_id = arun.id
    
           join data_item dit_in on
           dit_in.id = arin.data_item_id
 
           
            where arun.id =17832

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
 * 
 */ 
  
/**
 * Administration settings page for the algorithms.module
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
  $form['algorithm_service_host'] = array(
    '#type' => 'textfield',
    '#title' => t('Algorithm Service Hostname'),
    '#default_value' => variable_get('algorithm_service_host', 'http://localhost/wsdl'), 
    '#description' => t('This is the full URL prefix to the server hosting the algorithm web services.')
    );
  $form['algorithm_quarantaine_proxy'] = array(
    '#type' => 'textfield',
    '#title' => t('Algorithm Quarantaine Proxy Address'),
    '#default_value' => variable_get('algorithm_quarantaine_proxy', '0.0.0.0'), 
    '#description' => t('This is the IP address to the server hosting data for quarantained algorithm web services.')
    );
  $form['algorithm_quarantaine_netmask'] = array(
    '#type' => 'textfield',
    '#title' => t('Algorithm Quarantaine Netmask'),
    '#default_value' => variable_get('algorithm_quarantaine_netmask', '0.0.0.0'), 
    '#description' => t('This is the IP address netmask to determine whether an URL corresponds to the quarantained algorithm web services network.')
    );
   $form['algorithm_quarantaine_proxy_path'] = array(
    '#type' => 'textfield',
    '#title' => t('Algorithm Quarantaine Proxy Path'),
    '#default_value' => variable_get('algorithm_quarantaine_proxy_path', '/dae/spool/'), 
    '#description' => t('This is the path to the proxy data for quarantained algorithm web services.')
    );
   $form['algorithm_quarantaine_proxy_root'] = array(
    '#type' => 'textfield',
    '#title' => t('Algorithm Quarantaine Proxy Root'),
    '#default_value' => variable_get('algorithm_quarantaine_proxy_root', '/spool/'), 
    '#description' => t('This is the path to HTTPD root directory on the proxy web server containing the data for quarantained algorithm web services.')
    );
    
    return system_settings_form($form);
}

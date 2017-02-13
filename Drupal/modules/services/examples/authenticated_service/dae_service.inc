<?php
// $Id: $

/**
 * Access a DAE algorithm by wsdl.
 */

require_once("./".drupal_get_path('module','daedatabase')."/daedatabase_db.inc");
require_once("./".drupal_get_path('module','soap_server')."/nusoap/lib/nusoap.php");

global $db; 


/**
 * Call a remote web service.
 *
 * @param $name
 *   input parameters
 * @return
 *   algorithm outputs
 */ 
function dae_service_ocrad($name, $username, $password) {

	$user = user_authenticate(array('name' => $username, 'pass' => $password));
	
	if ($user->uid) {
		session_start();
		// $name is the input value for the effective service
		
		$client = new nusoap_client('http://rivendell.cse.lehigh.edu/dae/ocrad.php?wsdl', true);	
		// Access the effective services by using wsdl file. In future, we will get the service uri from database.
		
		// This is a test code to access database here.
		$db = getDAEDatabase();   

		$inputParas = $db ->q("select name from datatype where id in (
							select datatype_id from type_of where data_id in (
							select data_id from algorithm_input where algorithm_id = 166
							)
							)");
							
		$i = 1;
		$input = array();
		while($row = $db->fetch_array($inputParas)):
			
			$input[$row[NAME]] = $name[$row[NAME]];
			$i++;
			
		endwhile;
		
		// Get the service execution results.
		$result = $client->call('ocrad', array('name' => new soapval('name', 'complexInput', $name, false, 'urn:ocradwsdl'))); // call the operation of the effective service

		$result = array('ocrad_layout_result_file' => '111', 'ocrad_ocr_result_file' => '222');
		return $result; //output
	}
	
	session_destroy();
	//return services_error(t('Wrong username or password.'));
	return array('ocrad_layout_result_file' => 'Wrong username or password.', 'ocrad_ocr_result_file' => 'Wrong username or password.');
	
}

/**
 * Check if the user has permission to access the service
 */
function dae_service_ocrad_access($uid) {
  global $user;
  return (user_access('access ocrad'));
}


<?php
// $Id:  $

/**
 * Access a DAE algorithm by wsdl.
 */


/**
 * Call a remote web service.
 *
 * @param $name
 *   input parameters
 * @return
 *   algorithm outputs
 */ 
function dae_service_io($name) {

	return 'After service invocation: '.$name; //output
}

/**
 * Call a remote web service.
 *
 * @param $name
 *   input parameters
 * @return
 *   algorithm outputs
 */ 
function dae_service_o() {

	return 'Service invoked! '; //output
}

/**
 * Call a remote web service.
 *
 * @param $name
 *   input parameters
 * @return
 *   algorithm outputs
 */ 
function dae_service_noio() {
	$ourFileName = "testFile.txt";
	$ourFileHandle = fopen($ourFileName, 'w');
	fclose($ourFileHandle);

}

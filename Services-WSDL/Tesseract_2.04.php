<?php

/**
 * @file algowsdl.php
 * 
 * @author  Yingjie Li <yil308@lehigh.edu>
 * @author  Bart Lamiroy <lamiroy@cse.lehigh.edu>
 * 
 * @date October 2010
 * 
 * @version 1.0
 *
 * @section Main Description
 * 
 * This file is the principal stub for WSDL based Web Service providing.
 * It should be taken as a template for adaptation and extension of local
 * execution of DAE related services.
 * 
 * @section Usage - Disclaimer
 * 
 * The only part of this code that needs to be modified is the \a callback()
 * function. All other parts of this code should be left untouched. Relying on
 * personalized modification of the rest of the code may result in loss of 
 * compatibility with subsequent updates and evolutions of this code.
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
 */

/**
 * @mainpage
 * 
 * @section Contents
 * 
 * This project is composed of 3 files:
 *  - ocrad.php
 *  - client.php
 *  - setup.php
 * 
 * @section Note
 * 
 */
$algoOracleID = 125;
$algoname = 'Tesseract';
$algoversion = '2.04'; 
 
include('setup.php');

$inputT = array();
$inputT['input_image'] = array('name'=>'input_image', 'type'=>'xsd:string');
$outputT = array();
$outputT['text_file'] = array('name'=>'text_file', 'type'=>'xsd:string');



/**
 * This function is currently a demo stub running 'convert'. Eventually this will launch the 
 * effective execution of the corresponding algorithm (as set in the \a $input URIs array parameter)
 *  
 * @param [in] $input the array of arguments related to the execution
 * 
 * The exact input and output parameters of this function are defined by the \a $inputT and \a $outputT variables in \a setup.php.
 * 
 */
function callback($input) {

	global $htmlBaseDir;
	global $executionDir;
	global $executionPrefix;
	global $algoname;
	global $hostname;

	// Retrieving input file
	// ATTENTION! Be sure the array keys correspond to the keys declared in \a $inputT in setup.php
	$filename = $input['input_image'];
	$extension = '.txt';
	
	// Creating temporary work directory
	$localdir = $htmlBaseDir . '/' . $executionDir . '/' . $algoname . '/' . time();
	if (!is_dir($localdir) && ! mkdir($localdir,0777,true)) {
	    return new soap_fault('SERVER', '', 'Cannot create local directory', '');
	}
	
	// Copying the input file to the temporary directory
	$localname = tempnam($localdir, $executionPrefix);
	$localname = $localname . '_' . basename($filename);
	if(! copy($filename,$localname.'.tif')) {
	    return new soap_fault('SERVER', '', 'Cannot copy file', $filename);
	}
	/*
	 * This is currently only doing raw execution without the required logging
	 * on the DAE server.
	 */ 	
	$execString = '/dae/database/upload/351/tesseract '.escapeshellarg($localname.'.tif').' '.escapeshellarg($localname);
	$returnValue = system($execString,$returnCode);
	
	if ($returnCode) {
	    return new soap_fault('SERVER', '', 'Execution Error', 'Return code = ' . $returnCode);
	}
	
	// Stripping absolute path information and prefixing URL information.
	$localimage = 'http://'.$hostname. substr($localname.$extension,strlen($htmlBaseDir));
		
	// ATTENTION! Be sure the array keys correspond to the keys declared in \a $outputT in setup.php
	return array(
	   	'text_file' => $localimage
    );
}

/*
 * This PHP page genereates WSDL files for different algorithms with different inputs and outputs.
 */
 
// Pull in the NuSOAP code
require_once('lib/nusoap.php');

/**
 * Publishes a WSDL service for the algorithm defined in \a setup.php. 
 * It's WSDL interface description is returned. If not, it is supposed to be combined 
 * with an HTTP_POST upload of the required input data.
 * 
 * \todo UDDI registration on the DAE server.
 * 
 * THIS CODE SHOULD NOT BE MODIFIED WHEN ADAPTED TO SOME LOCAL SERVICE. 
 * ALL CODE MODIFICATION, CONFIGURATION AND ADAPTATION SHOULD BE DONE IN EITHER
 * \a setup.php OR IN THE ABOVE \a callback() FUNCTION.
 * 
 * Modification of the code below may result in incompatibility issues or difficulties
 * to upgrade to newer versions.
 */
 
// Create the server instance
$server = new soap_server();

// Retrieve the path of the algorithm to be invoked
//$algopath = $db->r("select PATH from ALGORITHM where ID = ".$algoid);
	
// Initialize WSDL support
$server->configureWSDL($algoname.'wsdl', 'urn:'.$algoname.'wsdl');

/*
 * Generate an array of input parameters as resulting from the following query
 * "select NAME from DATATYPE where ID in (	select DATATYPE_ID from TYPE_OF where DATA_ID in (
 *				select DATA_ID from ALGORITHM_INPUT where ALGORITHM_ID = ". $algoid ."	))"
 *
 * This array (\a $inputT) is defined in \a setup.php
 *  
 */ 
$server->wsdl->addComplexType('inputType_'.$algoname, 'complexType',	'struct',
							  'all', '', $inputT);

/*
 * Generate an array of output parameters as resulting from the following query
 * "select NAME from DATATYPE where ID in (	select DATATYPE_ID from TYPE_OF where DATA_ID in (
 *				select DATA_ID from ALGORITHM_OUTPUT where ALGORITHM_ID = ". $algoid ."	))" 
 *
 * This array (\a $outputT) is defined in \a setup.php
 *  
 */ 
$server->wsdl->addComplexType('outputType_'.$algoname, 'complexType', 'struct', 
							  'all', '', $outputT);

// Register the method to expose
$server->register('callback',                	// method PhP callback function
		array('args' => 'tns:inputType_'.$algoname),  	//input parameters
		array('return' => 'tns:outputType_'.$algoname), // output parameters
		'urn:'.$algoname.'wsdl',				// namespace              
		'urn:'.$algoname.'wsdl#'.$algoname,		// soapaction
   		'rpc',                                	// style
   		'encoded',                            	// use          
		$algoname.' algorithm'					// documentation
);

// Use the POST request to (try to) invoke the service
$HTTP_RAW_POST_DATA = isset($HTTP_RAW_POST_DATA) ? $HTTP_RAW_POST_DATA : '';
$server->service($HTTP_RAW_POST_DATA); 
?>

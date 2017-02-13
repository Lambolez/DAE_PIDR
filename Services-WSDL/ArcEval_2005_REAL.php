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
 * For demo executions, and if the \a $algoname variable is modified in setup.php, this file
 * should be renamed to \a $algoname.php.
 */
 
//== START EDITABLE ZONE 
/**
 * Edit the contents of setup.php to fit your needs.
 */ 
include('setup.php');

$algoname = 'ArcEval';
$algoversion = '2005';

$inputT = array();
$inputT['VEC_measured'] = array('name'=>'VEC_measured', 'type'=>'xsd:string');
$inputT['VEC_reference'] = array('name'=>'VEC_reference', 'type'=>'xsd:string');

$outputT = array();
$outputT['output_result_file'] = array('name'=>'output_result_file', 'type'=>'xsd:string');

//== STOP EDITABLE ZONE

/**
 * This function is currently a demo stub running 'Ocrad'. Eventually this will launch the 
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

	// Creating temporary work directory
	$localdir = $htmlBaseDir . '/' . $executionDir . '/' . $algoname . '/' . time();
	if (!is_dir($localdir) && ! mkdir($localdir,0777,true)) {
	    return new soap_fault('SERVER', '', 'Cannot create local directory', '');
	}
	
	//== START EDITABLE ZONE
	/*
	 * The following editable zone is made for retrieving the input parameters
	 * and preparing them (copying on disk, other preparatory manipulations) to
	 * be provided as input arguments to the actual executable.
	 * 
	 * In this case there is only one input parameter: the input file
	 * 
	 * ATTENTION! When modifying this code, be sure the array keys correspond to 
	 * the keys declared in \a $inputT in setup.php
	 */
	$filename1 = $input['file1'];
	$filename2 = $input['file2'];
	
	// Copying the input file to the temporary directory
	$localname1 = tempnam($localdir, $executionPrefix);
	$localname1 = $localname1 . '_' . basename($filename1);
	if(! copy($filename1,$localname1)) {
	    return new soap_fault('SERVER', '', 'Cannot copy file', $filename1);
	}
	
	$localname = tempnam($localdir, $executionPrefix);
	$localname2 = $localname2 . '_' . basename($filename2);
	if(! copy($filename2,$localname2)) {
	    return new soap_fault('SERVER', '', 'Cannot copy file', $filename2);
	}

	$localOutput = $localname. '.txt';
/*	$localLayout = $localname . '.ocr';*/
	
	/*
	 * This is code is only doing raw execution without the required logging
	 * on the DAE server. Logging provenance on the DAE server can only
	 * be done when the software and service are dully registered with
	 * the platform.
	 */ 
	
	$execString = "c:\ArcEval2005\ArcEval2005.exe $localname1 $localname2 >> $localOutput";
	//== STOP EDITABLE ZONE
	
	$returnValue = system($execString,$returnCode);
	if ($returnCode) {
	    return new soap_fault('SERVER', '', 'Execution Error', 'Return code = ' . $returnCode);
	}

	//== START EDITABLE ZONE
	/*
	 * This editable zone retrieves the results from the execution and creates the appropriate
	 * output data structure for retrieval by the caller.
	 * 
	 * In this particular case, the execution output consists of 2 files that have been stored
	 * in the execution directory. Since this directory is accessible by the webserver, we construct
	 * the appropriate URLs so that the client can download them.
	 */
	// Stripping absolute path information and prefixing URL information.
	$localOutput = 'http://'.$hostname. substr($localOutput,strlen($htmlBaseDir));
/*	$localLayout = 'http://'.$hostname. substr($localLayout,strlen($htmlBaseDir));*/
		
	// ATTENTION! Be sure the array keys correspond to the keys declared in \a $outputT in setup.php
	$result = array(
	   	'output_result_file' => $localOutput
/*       	'layout_result_file' => $localLayout*/
    );
    //== STOP EDITABLE ZONE
    
    return $result;
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

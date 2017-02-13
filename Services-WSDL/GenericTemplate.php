<?php

/**
 * @file GenericTemplate.php
 * 
 * @author  Bart Lamiroy <lamiroy@cse.lehigh.edu>
 * @author  Yingjie Li <yil308@lehigh.edu>
 * 
 * @date May 2011
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
 */
 
//== START EDITABLE ZONE 
/**
 * If running your web service locally, edit the contents of localsetup.php to fit your local configuration.
 */ 
$algoname = 'MyAlgorithm';

$algoversion = '1.0';
$algopath = '/opt/path_to_algorithm/algorithm';

// Do not edit the following two lines !
$algoFileName = str_replace(array(' ','/','\\'),'_',$algoname);
$algoname = htmlentities($algoname);

include('localsetup.php');

/**
 * Array of input variables for hosted algorithm.
 * 
 * \todo write an extensive explanation on how to format this part
 * 
 * IMPORTANT: due to SOAP restrictions, the 'name' string cannot contain whitespaces. 
 * This is an important constraint, given the assumed correspondence with the Oracle
 * data or data_type mapping.
 * 
 * @var $inputT
 */
$inputT = array();
$inputT['input_file'] = array('name'=>'input_file', 'type'=>'xsd:string');

/**
 * Array of output variables for hosted algorithm.
 * 
 * \todo write an extensive explanation on how to format this part
 * 
 * IMPORTANT: due to SOAP restrictions, the 'name' string cannot contain whitespaces. 
 * This is an important constraint, given the assumed correspondence with the Oracle
 * data or data_type mapping.
 * 
 * @var $outputT
 */
$outputT = array();
//$outputT['output_files'] = array('name'=>'output_files', 'type'=>'tns:StringArray');
$outputT['output_file'] = array('name'=>'output_file', 'type'=>'xsd:string');

//== STOP EDITABLE ZONE

/**
 * This function launches the effective execution of the corresponding algorithm (as set in the \a $input URIs array parameter)
 *  
 * @param [in] $input the array of arguments related to the execution
 * 
 * The exact input and output parameters of this function are defined by the \a $inputT and \a $outputT variables.
 * 
 */
function callback($input) {

	global $htmlBaseDir;
	global $executionDir;
	global $executionPrefix;
	global $algoname;
	global $hostname;
	global $algopath;

	// Creating temporary work directory
	$localdir = $htmlBaseDir . '/' . $executionDir . '/' . $algoname . '/' . time();
	if (!is_dir($localdir) && ! mkdir($localdir,0777,true)) {
	    error_log("Problem creating $localdir");
	    return new soap_fault('SERVER', '', 'Cannot create local directory', '');
	}
	
	/*
	 * Creating array $inputValues translating initial $input to local values by
	 * downloading remote input files localy.
	 */  
	$inputValues = array();
	foreach(array_keys($input) as $inputFieldName) {

	    if(is_array($input[$inputFieldName])) {
		$inputValues[$inputFieldName] = array();
	        foreach(array_keys($input[$inputFieldName]) as $inputItem) {
		    $inputValues[$inputFieldName][$inputItem] = getRemoteArguments($input[$inputFieldName][$inputItem]);
	        }
	    }
	    else {
	      $inputValues[$inputFieldName] = getRemoteArguments($input[$inputFieldName]);
	    }	

	}

	//== START EDITABLE ZONE
	/*
	 * The following editable zone is made to provide the input arguments to the actual executable.
	 * 
	 * ATTENTION! When modifying this code, be sure the array keys correspond to 
	 * the keys declared in \a $inputT and \a $outputT.
	 * 
	 * $localOutput is an array containing one entry for each return value defined in $outputT.
	 * 
	 */
	$localOutput['output_file'] = realpath($localdir) .  '/output.pbm';
    $logFile = escapeshellarg(realpath($localdir) .  '/logfile.txt');
	
	$execString = "$algopath " . escapeshellarg($inputValues['input_file']) . ' '. escapeshellarg($localOutput['output_file']) . " > $logFile 2>&1";
	//== STOP EDITABLE ZONE
	error_log("Executing $execString");

	$returnValue = system($execString,$returnCode);
	if ($returnCode) {
	    error_log("Execution failed: $returnValue");	
	    return new soap_fault('SERVER', '', 'Execution Error', 'Return code = ' . $returnCode);
	}

	//== START EDITABLE ZONE
	
	/*
	 * Placeholder for optional post-execution generation of \a $localOutput[] data
	 */
	
	//== STOP EDITABLE ZONE
	
	/*
	 * Transforming local output filenames into URLs when needed
	 */
	foreach(array_keys($localOutput) as $outputFieldName) {
	
	    if(is_array($localOutput[$outputFieldName])) {
	        foreach(array_keys($localOutput[$outputFieldName]) as $outputItem) {
	            if(is_file($localOutput[$outputFieldName][$outputItem])) {
        	        // Stripping absolute path information and prefixing URL information.
	                $localOutput[$outputFieldName][$outputItem] = 'http://'.$hostname. substr($localOutput[$outputFieldName][$outputItem],strlen(realpath($htmlBaseDir)));
	            }
	            
	        }
	    }
	    else {
	        if(is_file($localOutput[$outputFieldName])) {
        	    // Stripping absolute path information and prefixing URL information.
	            $localOutput[$outputFieldName] = 'http://'.$hostname. substr($localOutput[$outputFieldName],strlen(realpath($htmlBaseDir)));
	        }
	    }
	}
    
    return $localOutput;
}

function getRemoteArguments($argument) {

    $fragments = parse_url($argument);

    if($fragments['host']) {    
        $localname = tempnam($localdir, $executionPrefix);
        if(! copy($argument,$localname)) {
            return new soap_fault('SERVER', '', 'Cannot copy file', $argument);
        }
	$imageInfo = getimagesize($localname);
        $returnValue = $localname . '_' . basename($fragments['path']) . image_type_to_extension($imageInfo[2]);
        rename($localname,$returnValue);
    }
    else {
        $returnValue = $argument;
    }

  return $returnValue;
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

// StringArray
$server->wsdl->addComplexType('StringArray', 'complexType', 'array', '', 'SOAP-ENC:Array', array(),
                array(
                        array(
                                'ref' => 'SOAP-ENC:arrayType',
                                'wsdl:arrayType' => 'xsd:string[]'
                        )
        ), 'xsd:string'
        );

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
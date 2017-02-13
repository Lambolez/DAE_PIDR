<?php
// $Id: $

/**
 * Access a DAE algorithm by wsdl.
 */

require_once('./'.drupal_get_path('module','daedatabase')."/daedatabase_db.inc");
require_once('./'.drupal_get_path('module','dae_data')."/dae_data_person.inc");
require_once(variable_get('NuSOAP Path',"./".drupal_get_path('module','soap_server')."/nusoap/lib/nusoap.php"));
require_once('./'.drupal_get_path('module','algorithms')."/algorithms_status.var");
require_once('./sites/default/misc/identify.php');

global $db; 

/**
 * The following code was taken from http://php.net/manual/en/function.md5-file.php
 * 
 * Enter description here ...
 * @param unknown_type $fn1
 * @param unknown_type $fn2
 */
function files_identical($fn1, $fn2) {
    if(filetype($fn1) !== filetype($fn2))
        return FALSE;

    if(filesize($fn1) !== filesize($fn2))
        return FALSE;

    if(!$fp1 = fopen($fn1, 'rb'))
        return FALSE;

    if(!$fp2 = fopen($fn2, 'rb')) {
        fclose($fp1);
        return FALSE;
    }

    $same = TRUE;
    while (!feof($fp1) and !feof($fp2))
        if(fread($fp1, READ_LEN) !== fread($fp2, READ_LEN)) {
            $same = FALSE;
            break;
        }

    if(feof($fp1) !== feof($fp2))
        $same = FALSE;

    fclose($fp1);
    fclose($fp2);

    return $same;
}

/**
 * 
 * Enter description here ...
 * @param unknown_type $data
 * @param unknown_type $dataType
 * @param unknown_type $flag
 * 
 * \todo This function should also register a standard Copyright value.
 */
function createDataItem($data,$dataType,$flag) {
    
    $db = getDAEDatabase();
    
    $inserted_id = $db->createUniqueEntry('DATA_ITEM','ID');
    
    if($inserted_id) {
        $variables_update[':insertedid'] = $inserted_id;
        $variables_update[':flag'] = $flag;
    
        $variables_instance[':data'] = $data;
        $variables_instance[':insertedid'] = $inserted_id;

        $variables_associate[':datatype'] = $dataType;
        $variables_associate[':insertedid'] = $inserted_id;
    
        $db->i("update DATA_ITEM set FLAG =:flag, DESCRIPTION='Algorithm Generated' where ID = :insertedid",$variables_update);
        $db->i('insert into INSTANCE_OF values(:insertedid, :data)',$variables_instance);   
        $db->i('insert into ASSOCIATE_DATATYPE_DATA_ITEM values(:insertedid, :datatype)',$variables_associate);   
    
    	/** \todo Associate contributor information to newly generated data using the 'contribute' relation **/
    }
    
    return $inserted_id;
}

/**
 * This is a workaround to some restriction on the Solaris CSW PhP version, not being able
 * to have remote copy() execution.
 * This fuction copies \a $value (presumably remote, but not necessary) to \a $localname.
 * It first tries to invoke the standard copy() function. If that fails it makes a call to 
 * \a wget using a system() call.
 * 
 * @param [in] $value
 * @param [in] $localname
 * 
 * @returns Name of localy created file
 * 
 * \todo Make this function a bit more secure by checking the parameters correctly, as well 
 * as checking the correct execution and the result of each step.
 */
function localCopy($value,$localname) {
    
    $extension = preg_replace('@^.*(\.\w+)$@','$1',$value);
    if($extension == $value) {
        $extension = NULL;
    }
    
    $fullPathName = $localname;
	
    if($extension && $extension != preg_replace('@^.*(\.\w+)$@','$1',$localname)) {
        $fullPathName .=  $extension;
    }
    
    ini_set("allow_url_fopen","On");
	/** Note that the "allow_url_fopen = On" should be set in php.ini for copy to work accross the network 
	 *  (and even then you still can get surprises) **/
	 if(!copy($value,$fullPathName)) {	            
	    // A quick hack to force copying (e.g. on overly secured platforms)
	    /** \todo Get rid of this hard coded /opt/csw/bin path **/
	    system('/opt/csw/bin/wget -O '.escapeshellarg($fullPathName).' '.escapeshellarg($value));
	    error_log ("wget -P '$fullPathName' '$value'", 0);
	 }
	 
	 return $fullPathName;
}
/**
 * 
 * @param string $dataType
 * @param integer $dataTypeId
 * @param integer $dataId
 * @param mixed $value
 * @param boolean $remoteData
 */
function registerDataItem($dataType, $dataTypeId, $dataId, $value, $remoteData = false) {
    //error_log ( date('D, d M Y H:i:s') . ' ' . $dataType. ' ' . $dataId. ' ' . $dataTypeId. ' ' . $value . " ->  for Registration\n", 3, '/tmp/myphplog');
    
    $db = getDAEDatabase();
    
    $dataItemId = createDataItem($dataId,$dataTypeId,$dataType);
    
    switch($dataType) {
        case 'page_image':
            /** 
             * \todo All folowing code should be put in coherence with existing 
             * Oracle triggers and procedures used for uploading datasets
             */
            $db->i("insert into IMAGE_DATA_ITEM (ID) values($dataItemId)",array());  
            $db->i("insert into PHYSICAL_IMAGE_DATA_ITEM (ID) values($dataItemId)",array());  
            
            /*
             * Step 1: Generate the directory to store the result
             * Directory name is based on $dataId and the current date if the data is local.
             * If the data is remote ...
             */ 
            /** \todo Make '/dae/database' appear somewhere in the configuration variables **/
            $localdir = '/dae/database/' . variable_get('dae_db_name','test') . '-generated/' . $dataId . '/' . date('Y-z');
	        if(! is_dir($localdir)) 
	            mkdir($localdir,0777,true);
	
	        /*
	         * Step 2: generate a unique file name for the result (to avoid race conditions)
	         * and copy it. (if the standard copy() function fails, try and use 'wget'.
	         */
	        $localname =  uniqid ($localdir . '/output_', true);
	        
	        /** Note that the "allow_url_fopen = On" should be set in php.ini for copy to work accross the network 
	         *  (and even then you still can get surprises) **/
	        /** \todo Check parameters for malevolant actions like passing /etc/passwd **/
	        $fullPathName = localCopy($value,$localname);

	        /*
	         * Step 3; store it in the database
	         */
	        $imgSize = linpha_getimagesize($fullPathName);   
	        
	        /*
	         * Sometimes getimagesize fails on uncommon or too large images 
	         * In that case, we don't register width and heigth.
	         */
	        $variables[':fullpathname'] = $fullPathName;
	        
	        if(!$imgSize) {
	        	/** \todo check whether this insert does not return any error ! */ 
	            $db->i("insert into PAGE_IMAGE (ID,PATH) values($dataItemId,:fullpathname)",$variables);
	            
	        }
	        else {
	            /** \todo check whether this insert does not return any error ! */ 
	            $db->i("insert into PAGE_IMAGE (ID,PATH,WIDTH,HEIGHT) values($dataItemId,:fullpathname,".$imgSize[0].','.$imgSize[1].')',$variables);
	        }
	        
	        
	        /** \todo generate the thumbnails in the database **/
	        /** \todo Also store the 'mime' field obtained from getimagesize as a datatype associated to the page_image **/
	        
            break;
        case 'page_element':
            $db->i("insert into IMAGE_DATA_ITEM (ID) values($dataItemId)",array());  
            $db->i("insert into PHYSICAL_IMAGE_DATA_ITEM (ID) values($dataItemId)",array()); 
            /** \todo Manage page_element insertion after execution */ 
            break;
        case 'files':
            /**
             * \todo All folowing code should be put in coherence with existing 
             * Oracle triggers and procedures used for uploading datasets
             */
            
            /*
             * Step 1: Generate the directory to store the result
             * Directory name is based on $dataId and the current date.
             */ 
            /** \todo Make '/dae/database' appear somewhere in the configuration variables **/
            $localdir = '/dae/database/' . variable_get('dae_db_name','test') . '-generated/' . $dataId . '/' . date('Y-z');
	        if(!is_dir($localdir)) mkdir($localdir,0777,true);
	
	        /*
	         * Step 2: generate a unique file name for the result (to avoid race conditions)
	         * and copy it.
	         */
	         /** \todo Check parameters for malevolant actions like passing /etc/passwd **/
	        
	        $localname = uniqid ($localdir . '/output_', true);
	        $fullPathName = localCopy($value,$localname);
	
	        $variables[':fullpathname'] = $fullPathName;
	        $variables[':basepathname'] = basename($fullPathName);
	        /*
	         * Step 3; store it in the database
	         */
	        $db->i("insert into FILES (ID,NAME,PATH) values($dataItemId,:basepathname,:fullpathname)",$variables);
	        
            break;
        case 'dataset':
            $db->i("insert into IMAGE_DATA_ITEM (ID) values($dataItemId)",array());  
            $db->i("insert into LOGICAL_IMAGE_DATA_ITEM (ID) values($dataItemId)",array());  
            /** \todo Manage dataset insertion after execution */ 
            break;
        case 'page_element_property_value':
            $db->i("insert into IMAGE_DATA_ITEM (ID) values($dataItemId)",array());  
            $db->i("insert into LOGICAL_IMAGE_DATA_ITEM (ID) values($dataItemId)",array());  
            /** \todo Manage page_element_property_value insertion after execution */ 
            break;
        case 'argument':
            $variables[':value'] = $value;
            $db->i("insert into EXECUTION_DATA_ITEM (ID) values($dataItemId)",array());  
            $db->i("insert into ARGUMENT values($dataItemId,:value)",$variables);  
            break;
        default:
            error_log(date('D, d M Y H:i:s') . ' Unknown datatype: '.$dataType);
            return;
    }

    //error_log ( date('D, d M Y H:i:s') . ' ' . $dataItemId. " created\n", 3, '/tmp/myphplog');
    
    return $dataItemId;
}

/**
 * This function is called when a registered web service is executed. It is called once for each individual
 * input argument to the web service.
 * 
 * The input parameters of this fuction define the given input argument to the web service. It checks
 * whether the argument is already existant (registered) in the DAE database and returns its data_item.id
 * value if it is. The function returns -1 if the argument is currently unregistered in the database.
 * 
 * @param string $dataType contains the class of data (page_image, files, argument ...) as per datatype.data_item_type
 * @param integer $dataTypeId contains the ID of the datatype under consideration
 * @param integer $dataId contains the ID of the data under consideration
 * @param mixed $value contains the value of the data_item under consideration
 * @param integer $runId the Oracle database identifier of the currenr algorithm run
 * 
 * @returns the data_item.id value if the specified parameters correspond to an existing object in the DAE
 * database; returns -1 if they don't correspond to an existing object; returns NULL if an error occurs.
 */
function findDataItemToRegister($dataType, $dataTypeId, $dataId, $value,  $runId) {

    $db = getDAEDatabase();
    
    if($dataTypeId) {
        /*
         * The input argument to register has a datatype.
         */
        settype($dataTypeId, 'integer');
        settype($dataId, 'integer');

		$dataItemType = $db->r('select DATA_ITEM_TYPE from DATATYPE where ID='.$dataTypeId,array());

        /*
         * Making the assumption that the $value argument is either a URL (in which case it should 
         * correspond to a 'complex' dataType (files, page_image, dataset ...) or not. If it is not 
         * a URL, then the following call parse_url() should fail or not wield any 'host' value.
         * 
         * @bug this rationale does not hold when the $dataType is a string argument containing 
         * a URL.
         */
        $argumentParts = parse_url($value);
        $URLhostname = $argumentParts['host'];

        $hostname = (!isset($PHP_MAJOR_VERSION) || $PHP_MAJOR_VERSION < 5 || ($PHP_MAJOR_VERSION == 5 && $PHP_MINOR_VERSION <= 2) ? php_uname('n') : gethostname());
        
        if($URLhostname) {

            //* \bug compairing gethostname() can fail for multiple reasons, one being a legit proxy Drupal server,
            //* accessing the data on another legit server.             
            if(gethostbyname($URLhostname) != gethostbyname($hostname) &&
                $URLhostname != 'dae.cse.lehigh.edu') {
               // Probably a remote data_item (unless some weirdo used a strange multiple-IP server configuration)
                 

               /**
                * \todo find out what to do when algorithms are run on remote data and how to store the information
                * 
                * One of the possible things would be to check if the referenced image path (URL) is already stored
                * in the database. Register it if it isn't, and then use it as a local reference.
                * 
                * The major question here is 'Copyright'. Although we do not store the image locally (only the URL 
                * pointing to it) derived content may eventually get stored.
                * 
                * Also, there is no guarantee that the referenced URL remains accessible for the remaining duration
                * of the whole platform's life.
                * 
                */
                switch($dataItemType) {
		   			case 'files':
                   	case 'page_image':
						$isPresent = $db->r("select id from $dataItemType where path=:pathName order by id desc",array(':pathName' => $value));
						break;
		   			case 'dataset':
		   			default:
						error_log("Cannot handle remote $dataItemType parameters");
	                	return -1;
				}

				/** \todo Make '/dae/database' appear somewhere in the configuration variables **/
				$localFileName = '/dae/database/' . variable_get('dae_db_name','test') . "-cache/$isPresent_" . str_replace('..','_',$value) ;

				if($isPresent) {
				/* If remote object is already registered, check cache for possible modification. */
					if(!is_file($localFileName. "_$isPresent")) {
						error_log("$localFileName inexistant!");
						return -1;
					}
					else {
			   			/* 1. download file to temporary location */
						$tmpFileName = tempnam('/tmp', 'web_');
						$genuineTmpFileName = localCopy($value, $tmpFileName);
			   			/* 2. compare cache file with temporary file */
						if(!files_identical($genuineTmpFileName,$localFileName)) {
			   				/* 3. if files differ, register new version of file */
						}	
			   		/* 4. return id of latest version */
			   		/* 5. do the verification/update stuff done for registered dataitems */
					}
                }
				else {
			   		/* 1. download file to temporary location */
			   		/* 2. register file */	
			   		/* 3. copy file to /dae/database/XXX-cache */
			   		/* 4. return id  */
			   		/* 5. do the verification/update stuff done for registered dataitems */
				}
            }
            else {
                // This is a locally referenced data_item (suposedly present in the database).
                /* First - check if the URL is valid */
                $fullPath = $argumentParts['path'].$argumentParts['query']; // concatenating is necessary since Drupal allows two ways of referencing using CleanURLs
                
                $foolProofExistenceCheck=false;
                if(preg_match('@/(\d+)$@',$fullPath,$match)) {
                    //error_log ( date('D, d M Y H:i:s') . ' ' . print_r($match[1],true) . " (match) $fullPath \n" , 3, '/tmp/myphplog');
                    $dataItemId = $match[1];
                    /*
                     * If the following query returns a value, this means that the argument has already been successfully registered
                     * as an input of equivalent algorithm runs.
                     */
                    $foolProofExistenceCheck = ($db->r('
                    	select count(DI.ID)
                    	from 
                    		DATA DT,
                    		DATATYPE DTT,
                    		DATA_ITEM DI,
                        	ASSOCIATE_DATATYPE_DATA_ITEM ASDD,
                        	INSTANCE_OF IO
                    	where 
                    		DI.ID='.$dataItemId.' and
                    		DTT.ID='.$dataTypeId.' and
                        	DI.ID = ASDD.DATA_ITEM_ID and
                    		DTT.ID = ASDD.DATATYPE_ID and
                    		DT.ID = IO.DATA_ID and
                    		DI.ID = IO.DATA_ITEM_ID
                    	',array()) > 0);
                }
                else {
                    // Invalid URL format
                    //* \todo raise exception and rollback algorithm run execution
                    return;
                }
                /** Second - \todo check its type validity */
                if($foolProofExistenceCheck)
                    return $dataItemId;
                else {
                    // Check whether the data_item exists
                    $existenceCheck = ($db->r('select count(ID) from DATA_ITEM where ID='.$dataItemId,array()) > 0);
                
                	if(! $existenceCheck) { // If it doesn't: FAIL !
                	    return -1;
                	} else { 
                	    /*
                	     * If we're here, that means that:
                	     * 1. $foolProofExistenceCheck was false
                	     * BUT
                	     * 2. $existenceCheck is true.
                	     * 
                	     * The data_item exists, but the registered meta-data does not guarantee that
                	     * it is fully type compatible with the algorithm input requirements.
                	     */
                	    
                	    // Check whether the data_item has the required data_type
                	    $dataTypeCompatibility =  $db->r('
                	    	select 
                	    		count(DTT.ID)
                    		from 
                    			DATATYPE DTT,
                    			DATA_ITEM DI,
                            	ASSOCIATE_DATATYPE_DATA_ITEM ASDD
                    		where 
								DI.ID='.$dataItemId.' and
                    			DTT.ID='.$dataTypeId.' and
                    			DI.ID = ASDD.DATA_ITEM_ID and
                    			DTT.ID = ASDD.DATATYPE_ID',array());
                	    
                	    // Check whether the data_item has already been registered as the required data
                	    $dataCompatibility =  $db->r('
                	    	select 
                	    		count(DT.ID)
                    		from 
                    			DATA DT,
                    			DATA_ITEM DI,
                            	INSTANCE_OF IO
                    		where 
								DI.ID='.$dataItemId.' and
                    			DT.ID='.$dataId.' and
                    			DT.ID = IO.DATA_ID and
                    			DI.ID = IO.DATA_ITEM_ID',array());
                	    
                	    // If one of the above queries was successful, proceed by returning the data_item's Id
                	    if($dataTypeCompatibility || $dataCompatibility) {
                	        return $dataItemId;
                	    }
                	    else {
                	        /**
                	         * The data_item is present in the database but has no registered links with the required data or datatype values
                	         * 
                	         * \todo decide on a coherent policy to handle strangely referenced inputs.
                	         * 
                	         * Currently we'll just log a message and return the id
                	         */
                	        
                	        // Check whether the data_item is stored in compatible table
                	        $variables_compat[':datatype'] = $dataType;
                	        $variables_compat[':dataitemid'] = $dataItemId;
                	        
                	        $tableCompatibility =  $db->r('
                	    		select 
                	    			count(DI.ID)
                    			from 
	                    			DATA_ITEM DI
            	        		where 
									DI.ID=:dataitemid and
                    				DI.FLAG=:datatype',$variables_compat);
                	        
                	        if(!$tableCompatibility)
                	            error_log ( "Unveryfiable input type for data_item $dataItemId : $dataType - $dataId - $dataTypeId - $value");
                	            
                	        return $dataItemId;
                	    }
                	    
                    	/* Final - return error id (this line of code should never be reached) */
                        return -1;
                    }
                }            
            }
        }
        else {
            // Argument is a value and not an URL
            
            /*
             * We currently only handle this case for simple type 'argument' values
             * ('argument' as per the data model in the database)
             */
            $variables[':value'] = $value;
            
            switch($dataType) {
                case 'argument':
                    $argumentId =  $db->r('
                	    	select 
                	    		AGT.ID
                    		from 
                    			ARGUMENT AGT
                    		where 
								AGT.VALUE=:value',$variables);
                    
                    if(!$argumentId) {
                        /*
                         * If the argument does not exist. Create it!
                         */
                        $db->i("insert into DATA_ITEM (ID, STATUS, DESCRIPTION, COPYRIGHT, FLAG) values (seq_data_item.nextval, 0, 'Web Service Generated', '', 'argument')",array());
	                    $argumentId = $db->r("SELECT seq_data_item.CURRVAL FROM DUAL",array());
	                    $db->i("insert into EXECUTION_DATA_ITEM (ID) values ($argumentId)",array());
	                    $db->i("insert into ARGUMENT (ID,VALUE) values ($argumentId,:value)",$variables);
                        
	                    $db->i("insert into ASSOCIATE_DATATYPE_DATA_ITEM (DATA_ITEM_ID, DATATYPE_ID) values($argumentId,$dataTypeId)", array());
	                    
	                    $typeCoherence = $db->r("select count(*) from TYPE_OF where DATA_ID = $dataId and DATATYPE_ID = $dataTypeId");
	                    if($typeCoherence != 1) {
	                        error_log("Invalid type coherence in TYPE_OF: DATA_ID = '$dataId', DATATYPE_ID = '$dataTypeId'");
	                        if($typeCoherence == 0) {
	                          $db->i("insert into TYPE_OF (DATA_ID, DATATYPE_ID) values($dateId,$dataTypeId)", array());
	                        }
	                    } 
	                        
                    }
              
                    return $argumentId;
                 
                    break;
                    
                case 'dataset':
                	if(!is_array($value)) {
                		error_log ( "Raw values for $dataType : must be Arrays not '$value'");
                		return;
                	}
         			/* continue to default handling: cannot handle raw datasets yet */
                default:
                    error_log ( "Cannot handle raw values for $dataType : $dataId - $dataTypeId - $value");
                    return;
            }
        }
	    
    } 
    else {
        /**
         * This configuration should be impossible ... how can a registered algorithm argument not belong to a valid 'dataType' ?
         * 
         * Currently we're assuming in this case that this means that the argument is of type 'argument'
         * 
         * \todo implement the 'argument' insertion when running an algorithm.
         * \todo raise an exception
         */
        //error_log ( date('D, d M Y H:i:s') . ' ' . $value . " - not referenced in 'data' table ?? \n", 3, '/tmp/myphplog');        
    }
    
}

/**
 * Registers all provenance information with respect of the staring of a run of the algorithm 
 * defined by \a $algoId and with input parameters \a $callParameters.
 * 
 * This function will check if the $callParameters are already present in the DAE Oracle database,
 * and will update the following tables:
 *  - ALGORITHM_RUN
 *  - ALGORITHM_RUN_OF
 *  - EXECUTES_RUN
 *  - ALGORITHM_RUN_INPUT
 *  - INPUT_CONFIGURATION
 *  
 * @todo Currently, the INSTANCE_OF table is not updated.
 * @todo Currently, input data of type \a dataset, \a page_element, \a page_element_property_value are not
 * handled appropriately (actually ... they are ignored)
 * 
 * If execution is to be quarantained, the input data referenced in \a $callParameters is copied
 * to the quarantaine exchange area, and paths/URLs in \a $callParameters are updated accordingly. 
 * 
 * @param [in] $algoId \a string indentifier of algorithm being run
 * @param [in,out] $callParameters \a mixed associative array of input values, indexed by DATA.id
 * @param [in] $quarantainedExecutionProxy \a boolean (default = \a NULL) flag indicating if execution is quarantained
 *  
 */
function registerProvenanceStart($algoId, &$callParameters, $quarantainedExecutionProxy = null) {
    
   	$db = getDAEDatabase();
   	settype($algoId, 'integer');
   	
   	/* Retrieving all DATA.id values serving as input for current algorithm 
   	 * and creating a new run in the DAE Oracle database ($inserted_id) */
	$alg_input = $db->q("select DATA_ID from ALGORITHM_INPUT where ALGORITHM_ID = ".$algoId,array());
	$inserted_id = $db->createUniqueEntry('ALGORITHM_RUN','ID');
	
	$webDescription = 'WebService Initiated Run';
	$insertDate = date($db->timestampFormat); 
	
	global $user;
	
	/*
	 * Generating algorithm run metadata and storing it in the DAE Oracle database
	 * - First storing generic provenance data
	 * - Second looping through input parameters DATA.id values from previously retrieved $alg_input.
	 */
	$variables_update[':webdescription'] = $webDescription;
	$variables_update[':insertdate'] = $insertDate;
	$variables_update[':insertedid'] = $inserted_id;
	
	$variables_runof[':algoid'] = $algoId;
	$variables_runof[':insertedid'] = $inserted_id;
	
	$variables_executes[':userid'] = (isset($user->uid) ? getPID($user->uid) : 1);
	$variables_executes[':webdescription'] = $webDescription;
	$variables_executes[':insertedid'] = $inserted_id;
	
	$db->i('update ALGORITHM_RUN set DESCRIPTION =:webdescription, START_TIME =:insertdate, STATUS = '.RUN_RUNNING.' where ID = :insertedid',$variables_update);
	$db->i('insert into ALGORITHM_RUN_OF values(:insertedid, :algoid)',$variables_runof);
	$db->i('insert into EXECUTES_RUN values(:userid, :insertedid, :webDescription)',$variables_executes);
	
	$i = 0;
	/*
	 * Looping through the current algorithm's input parameters, identified by their DATA.id
	 */
	while($row = $db->fetch_array($alg_input)) {

	    $dataId = $row['DATA_ID'];
	    
	    error_log("Registering $dataId for $algoId");
	    /* Retrieving DATATYPE.id and DATA_ITEM_TYPE for current input parameter */
	    /**
	     * \todo We're assuming here there will be at most one row returned by this query (i.e. any data has at most one datatype)
	     * We should check if this is always true !
	     */ 
        $dataType = $db->fetch_array($db->q('select ID, DATA_ITEM_TYPE from DATATYPE, TYPE_OF TOF where ID = TOF.DATATYPE_ID and TOF.DATA_ID='.$dataId,array()));
	    
	    if($dataType) {
	        /* 
	         * Checks whether the current input argument is a stored item in the database or not
	         * If it is, returns its DATA.id. 
	         */
	        $argId = findDataItemToRegister($dataType['DATA_ITEM_TYPE'], $dataType['ID'], $dataId, $callParameters[$dataId], $inserted_id);

	        /* 
	         * In the case of quarantained execution, the input data needs to be copied to the proxy server 
	         * (independantly of whether it is local or not)
	         * 
	         * The following code creates the appropriate directory on the proxy volume, copies the data and
	         * modifies the callParameter values to include the paths to the proxy.
	         */
	        if($quarantainedExecutionProxy) {
	            
	            $proxyPath = variable_get('algorithm_quarantaine_proxy_path', '/dae/spool/').'orchestrator/uploads/'.$inserted_id;
	            
	            if(!is_dir($proxyPath)) {
	                mkdir($proxyPath,0777,true);
	            }
	            
	            switch ($dataType['DATA_ITEM_TYPE']) {
                    case 'page_image':
                    case 'files':
                        /* argument MUST be an URL */
                        
                        /*
                         * Copying original argument to proxy spool directory
                         */
                        $proxyName = "$dataId";
                        if ($argId > 0) {
                            $proxyName .= '_'.$argId;
                        } else {
                            $proxyName .= '_foreign';   
                        }
                        
                        $localCopy = localCopy($callParameters[$dataId], $proxyPath . '/' . $proxyName);
                        /*
                         * Renaming the callParameter argument as to point to the new URL.
                         */
                        $newPath = str_replace(variable_get('algorithm_quarantaine_proxy_path', '/dae/spool/'),'http://'. $quarantainedExecutionProxy . variable_get('algorithm_quarantaine_proxy_root', '/spool/'),$localCopy);
                        error_log('Quarantained execution: converting '. $callParameters[$dataId] . " to $newPath");
                        $callParameters[$dataId] = $newPath;
                        break;
                    case 'dataset':
                    case 'page_element':
                    case 'page_element_property_value':
                        /* argument CAN be an URL */
                        break;
                    case 'argument':
                        /* argument CANNOT be an URL */
                        break;
                    default:
                        /* should not occur ! */
                        break;
                }
	        }
	    }
	    else { 
	        $argId = findDataItemToRegister('argument',null,$dataId,$callParameters[$dataId],$inserted_id);
	    }

	    if($argId > 0) {
	        /** \todo Adapt to new data model with input_configuration and remove references to ALGORITHM_RUN_INPUT (obsoleted) **/
	        $db->i("insert into ALGORITHM_RUN_INPUT (ALGORITHM_RUN_ID, DATA_ITEM_ID) values ($inserted_id,$argId)",array());
	        $db->i("insert into INPUT_CONFIGURATION (ALGORITHM_RUN_ID, DATA_ITEM_ID, DATA_ID) values ($inserted_id,$argId,$dataId)",array());
	        
	        /**
	         * \todo Check existence in INSTANCE_OF table and update with following instruction if necessary (i.e. not already present)
	         */
	        //$db->i("insert into INSTANCE_OF VALUES($dataItemId, $dataId)");
	    }
	    
	    $i++;
    }

	db_query("insert into {runs_deleted} values($inserted_id,0)");
	
	return $inserted_id;
}

/**
 * 
 * @param [in] $insertedId
 * @param [in] $algoId
 *
 */
function registerProvenanceFailure($inserted_id, $algoId) {
    $db = getDAEDatabase();
    
    $variables[':insertdate'] = date($db->timestampFormat);
    $variables[':insertedid'] = $inserted_id;
     
	$db->i('update ALGORITHM_RUN set END_TIME = :insertdate, STATUS ='.RUN_FAILED. ' where ID = :insertedid',$variables);
}

/**
 * 
 * @param [in] $insertedId
 * @param [in] $algoId
 * @param [in/out] &$callParameters
 * 
 * \todo Getting this function right is far more complex than it might seem. 
 * One has to take into account two kinds of situations. The first case concerns
 * all-local input data; in which case output data should be stored into the data base
 * with full provenance. The second case is partially local data or completely external
 * data (non-locally hosted), in which case it is tricky to store the results and full
 * provenance (mainly for Copyright reasons).
 */
function registerProvenanceStop($inserted_id, $algoId, &$callParameters) {
    	
	//error_log ( date('D, d M Y H:i:s') . ' ' . print_r($callParameters,true) . "\n", 3, '/tmp/myphplog');
    
    $db = getDAEDatabase();
    
    settype($inserted_id, 'integer');
    $variables[':insertdate'] = date($db->timestampFormat);
    $variables[':insertedid'] = $inserted_id;
    
    $variables2[':algoid'] = $algoId;
    
	$db->i('update ALGORITHM_RUN set END_TIME = :insertdate, STATUS ='.RUN_COMPLETE.' where ID = :insertedid',$variables);
	
	$alg_output = $db->q('select DATA_ID from ALGORITHM_OUTPUT where ALGORITHM_ID = :algoid',$variables2);

	$i = 0;
	while($row = $db->fetch_array($alg_output)) {

	    $dataId = $row['DATA_ID'];
	    /**
	     * \todo We're assuming here there will be at most one row returned by this query (i.e. any data has at most one datatype)
	     * We should check if this is always true !
	     */ 
        $dataType = $db->fetch_array($db->q('select ID, DATA_ITEM_TYPE from DATATYPE, TYPE_OF TOF where ID = TOF.DATATYPE_ID and TOF.DATA_ID='.$dataId,array()));
        
	    //error_log ( date('D, d M Y H:i:s') . ' ' . print_r($dataType,true) . "\n", 3, '/tmp/myphplog');
	    
	    if($dataType) {
	        $argId = registerDataItem($dataType['DATA_ITEM_TYPE'], $dataType['ID'], $dataId, $callParameters[$dataId]);
	    }
	    else { 
	        $argId = registerDataItem('argument',null,$dataId,$callParameters[$dataId]);
	    }

	    if($argId > 0) {
	        /** \todo Adapt to new data model with input_configuration and remove references to ALGORITHM_RUN_OUTPUT (obsoleted) **/
	        $db->i("insert into ALGORITHM_RUN_OUTPUT (DATA_ITEM_ID, ALGORITHM_RUN_ID) values ($argId,$inserted_id)",array());
	        $db->i("insert into OUTPUT_CONFIGURATION (ALGORITHM_RUN_ID, DATA_ITEM_ID, DATA_ID) values ($inserted_id,$argId,$dataId)",array());
	        
	        $callParameters[$dataId] = url('<front>',array('absolute' => true)) . 'browse/dataitem/download/'.$argId;
	        //$callParameters[$dataId] = preg_replace('@^(.*)localhost(.*)$@','${1}127.0.0.1$2}',$callParameters[$dataId]);
	        //error_log ( date('D, d M Y H:i:s') . ' Setting URL to' . $callParameters[$dataId] . "\n", 3, '/tmp/myphplog');
	        /** \todo Check existence in INSTANCE_OF table and update with following instruction if necessary (i.e. not already present) */
	        //$db->i("insert into INSTANCE_OF VALUES($dataItemId, $dataId)");
	    }
	    else error_log('Failed to register data_item');
	    
	    $i++;
    }
	
}
/**
 * Looks for and calls hooks for the specified algorithm based on both
 * the name of the algorithm and the algorithm's ID.
 *
 * Example:
 * For $aid = 124 and $aname = "Kanungo Degradation_1.0", this function will
 * call hook_124 AND hook_kanungo_degradation_1_0.  Keep this in mind when
 * writing functions.
 *
 *
 * @param [in] $aid   - the integer ID of the algorithm
 * @param [in] $aname - the name of the algorithm with _<version>
 * @param [in] $args1 - argument(s) for the hook function hook_<name>
 						and hook_<id> if $args2 is not specified
 * @param [in] $args2 - argument(s) for the hook function if different
 *						for hook_<id>
 * 
 * @return true if a hook is called, false if no hook is found
 *
 */ 
function hook_algorithm($aid, $aname, $args1, $args2 = null){
	
	require_once('dae_service_algorithms_helper_functions.inc');
	
	$found = true;

	/* try a function with hook by name
	   format: $aname = "Kanungo Degradation_1.0" will call "hook_kanungo_degradation_1_0"
	   (replace all non-alphanumeric with _, make lower case and add hook_)
	   
	   NOTE: Example: $aname is set to "Kanungo Degradation_1.0"
	   - Should be utilized when possible to avoid inconsistencies in algorithm IDs
	     between production and sandbox servers.
	*/	
	$functionName = "hook_" . strtolower(preg_replace("/[^a-zA-Z0-9\s]/", "_", $aname));
	error_log("Calling hook $aid - $aname with '" . print_r($args1,true). "' as $functionName");
	
	if(function_exists($functionName))
		$functionName($args1);
	else
		$found = false;
	
	/* try a function with hook by id
	   format: $aid = 124 will call "hook_124"
	   
	   NOTE:
	   -Beware of hard coded IDs not matching on different databases
	*/
	$functionName = "hook_" . $aid;
	
	if(function_exists($functionName)){
		if(!empty($args2))
			$functionName($args2);
		else
			$functionName($args1);
		$found = true;
	}

	error_log("Hook $found");
	
	return $found;
}

/**
 * This function will check if the ALGORTITHM_RUN with id \a $runId has a 'clone'
 * (i.e. the execution of the same algorithm with the exact same input parameters)
 * 
 * @param [in] $runId
 * 
 * @returns the ALGORTITHM_RUN.id of a clone if one exists
 * 
 * Behavior is undefined if multiple clones exist, but a valid id will allways be returned
 * in that case.
 */
function checkForCloneExecution($runId) {

    $db = getDAEDatabase();

    /* 
     * Retrieve the ALGORITHM.id the current run is an execution of, as well as 
     * its number of input arguments.
     */
    $getitems = $db->q("select count(DATA_ID) as NBINPUT, ALGIN.ALGORITHM_ID as ALGOID
    					  from ALGORITHM_INPUT ALGIN
    					  join ALGORITHM_RUN_OF AROF on ALGIN.ALGORITHM_ID = AROF.ALGORITHM_ID
    					  where ALGORITHM_RUN_ID = $runId
    					  group by ALGIN.ALGORITHM_ID", array());
    
    $algoData = $db->fetch_array($getitems);
	
    $algoId = $algoData['ALGOID'];
    $nbInput = $algoData['NBINPUT'];
    
    /*
     * The following query will retrieve all ALGORITHM_RUN.id values for which the the input
     * parameters were exactly the same as those of the current run.
     * 
     */
    
    /**
     * @todo Finding the clone executions is currently only partially working. Since the framework
     * doesn't register foreign input parameters, and since we can have optional parameters as well, 
     * there is an ambiguity as to know whether an algorithm was run with default arguments or whether
     * some of these arguments were external parameters.
     */
    
    $getitems = 
    $db->q("select count(RUN2.DATA_ITEM_ID) as NBARGS, RUN1.ALGORITHM_RUN_ID as R1ID 
    		from INPUT_CONFIGURATION RUN1 
    		join INPUT_CONFIGURATION RUN2 
    			on RUN1.DATA_ITEM_ID = RUN2.DATA_ITEM_ID
    	    where RUN1.DATA_ID = RUN2.DATA_ID 
    	      and RUN1.ALGORITHM_RUN_ID <> RUN2.ALGORITHM_RUN_ID
    	      and RUN2.ALGORITHM_RUN_ID = $runId
    	      and RUN1.ALGORITHM_RUN_ID in (select ALGORITHM_RUN_ID
    	      								 from (select count(INCONF.DATA_ID) as NBPARAM, INCONF.ALGORITHM_RUN_ID 
    	      								 		from INPUT_CONFIGURATION INCONF
    	      								 	 	join ALGORITHM_RUN_OF AROF on AROF.ALGORITHM_RUN_ID = INCONF.ALGORITHM_RUN_ID
    	      								 	 	where AROF.ALGORITHM_ID = $algoId 
    	      								 	 	group by INCONF.ALGORITHM_RUN_ID having count(INCONF.DATA_ID) = $nbInput)
    	      								 ) 
    	    group by RUN1.ALGORITHM_RUN_ID having count(RUN2.DATA_ITEM_ID) = $nbInput", array());
    
    $clone = $db->fetch_array($getitems);
    return $clone['R1ID'];
}

/**
 * This is a generic callback through which all web-service calls are routed.
 * 
 * This function relies on a session variable $_SESSION['algoId'] to determine
 * which is the actual algorithm web service it will eventually call.
 * 
 * The session variable is set in dae_service_algorithms_auth() at the invocation
 * of the service.
 * 
 * This callback function prepares all input and output data to conform to the DAE
 * data model standards, registers provenance and reformats data to fit with remote
 * or quarantained services.
 *
 * @param [in] $callbackArguments
 * @param [in] $username
 * @param [in] $password
 * 
 * @return
 *   algorithm outputs
 */ 
function callback($callbackArguments = array(), $username, $password) {
	
    $algoId = $_SESSION['algoId'];
    
    global $user;
    
    $user = user_authenticate(array('name' => $username, 'pass' => $password));
    
    if ($user->uid && user_access('run algorithms') && user_access('access services')) {
        session_start();

        $db = getDAEDatabase();
        /* 
         * Get data related to algorithm to be invoked:
         * 	$algoString: format of supposed .php filename
         *  $algoHTMLString: $algoString sanitized for HTML use
         */
        $algoInfo = retrieveAlgortithmInfo($algoId);
        
        $algoString = $algoInfo['NAME'].'_'.$algoInfo['VERSION'];
        $algoHTMLString = str_replace(array(' ','/','\\'),'_',$algoString);
        $algoHTMLString = htmlentities($algoHTMLString);
	
        /**
         * If the algorithm is a standard, DAE hosted application:
         * 		generate the access path URL by using the algorithm_service_host Drupal variable and $algoHTMLString
         * 
         * @WARNING! 
         * This may be the source of difficult to identify bugs.
         * When the following error message is logged 'wsdl error: Bad path to WSDL file XYZ' this may mean that the 
         * $algoInfo['PATH'] value stored in the database, is wrong (e.g. due to case sensitivity or typos)
         * 
         */
        if($algoInfo['PATH'] == $algoInfo['NAME'] || preg_match ('@^/dae/database@' , $algoInfo['PATH'])) {
            $clientPath = variable_get('algorithm_service_host', 'http://localhost/wsdl/') . '/' .$algoHTMLString.'.php?wsdl';
        }
        /**
         * If the algorithm is NOT a standard, DAE hosted application, it is either a remotely hosted application
         * or a quarantained DAE hosted application.
         */        
        else { 
            /*
             *  Making the assumption that if the algorithm isn't local, its pathname is a valid URL
             *  (this is a reasonable assumption since $algoInfo['PATH'] comes directly from the DAE 
             *  Oracle database and it's resonably safe to assume it has been created using controlled
             *  procedures)
             *  
             *  In that case, the URL either is foreign (not hosted on the local infrastructure) or is
             *  within the non routable quarantaine network. In the case of the latter, subsequent input 
             *  argument URLs will need to be made available to the quarantaine proxy host, since 
             *  quarantained hosts have no access to external network resources.
             *  
             */
            $clientPath = $algoInfo['PATH'];
            
            /**
             * The algorithm is a quarantained DAE hosted application if its URL lies within
             * the defined quarantaine network.
             */
            $privateNetMask = variable_get('algorithm_quarantaine_netmask', '0.0.0.0');            
            $executionHostName = parse_url($clientPath,PHP_URL_HOST);
            if(ip2long($executionHostName) & ip2long($privateNetMask)) {
                $quarantaineExecution = true;
            }
            else {
                $quarantaineExecution = false;
            }
        }
        
        /* defining the client to the rebound webservice, by setting timeout to 300 seconds */
        $client = new nusoap_client($clientPath, true, false, false, false, false, 0, 300);

        /** \todo Get the nusoap error handling right and raise appropriate exceptions ... everywhere (not only here) ! */
        $err = $client->getError();
        if ($err) {
            error_log ($err);
            return new soap_fault('SERVER', '', 'Internal Client Initialization Error', print_r($err,true));
        }

        /** \todo the database retrieved Input/Output parameters should be checked to the published WSDL information */

        /*
         * Generating the input parameters for the proxy webservice by parsing the input passed to
         * the current function ($callbackArguments) and generating canonical names and stuctures by making
         * some reasonable but hard assumptions on how data structures and naming conventions
         * have been applied.
         */
        /** 
         * While the current callback's WSDL input/output description (and subsequent hierarchical structure 
         * of the $callbackArguments associative array) has a complex naming scheme combining DATA.name, DATA.id 
         * as well as DATA_TYPE.name and DATA_TYPE.id values, the actual webservice executing the algorithm has
         * a 'flat' WSDL input/output structure in which the argument names identical to DATA.name.  
         */
        /*
         * The complex multi-level $callbackArguments associative array is decomposed into:
         */
        $nameIndexedArguments = array(); /* a single-level associative array indexed by DATA.name */
        $dataIdIndexedArguments = array(); /* identical to $nameIndexedArguments but indexed by DATA.id */
        $idToNameAssociation = array(); /* array associating $dataIdIndexedArguments indexes (DATA.id) to $nameIndexedArguments indexes (DATA.name) */
        
        error_log("$algoString Callback argument parsing ", print_r(array_keys($callbackArguments),true));
        
        foreach(array_keys($callbackArguments) as $argumentName) {

            /** \todo We're assuming all array keys are unique ... check if this is actually the case 100% of the time */
            $argumentValue = $callbackArguments[$argumentName];
            $splitFields = array();
            
            /*
             * We're supposing that canonical argument names are of type 'Name_ID'
             * where 'Name' is the actual name of the argument and 'ID' is the DAE
             * Oracle DATA.id value of the argument.
             * 
             * We parse $argumentName to produce
             * $strippedArgumentName and $argumentDataId
             */
            preg_match('/(.*)_([\d]*)$/',$argumentName,$splitFields);
            $strippedArgumentName = $splitFields[1];
            $argumentDataId = $splitFields[2];

            /* Either the $callbackArgument has an associated DATA_TYPE - in which case its $argumentValue will be an array */ 
            if(is_array($argumentValue)) {
                /** \todo We're assuming composite types only have one component ... check if this is actually the case 100% of the time */
                $values = array_values($argumentValue);
                $nameIndexedArguments[$strippedArgumentName] = $values[0];
                $dataIdIndexedArguments["$argumentDataId"] = $values[0];
            }
            /* Either the $callbackArgument has NO associated DATA_TYPE - in which case its $argumentValue will be a plain value */ 
            else {
                $nameIndexedArguments[$strippedArgumentName] = $argumentValue;
                $dataIdIndexedArguments["$argumentDataId"] = $argumentValue;
            }
            
            $idToNameAssociation["$argumentDataId"] = $strippedArgumentName;
        }

        /**
         * Once all input arguments have been retrieved and have been associated wth their corresponding DATA.id
         * we resister a new ALGORITHM_RUN.id ($runId) and store and associate input provenance in the DAE Oracle
         * database.
         * 
         *  If the run is quarantained, we also need to copy the input to the quarantaine proxy and 
         *  translate input value URLs to this new location so that the quarantained web services can
         *  access the data.
         */
        error_log("$algoString registering start");
        $runId = registerProvenanceStart($algoId,$dataIdIndexedArguments,($quarantaineExecution) ? variable_get('algorithm_quarantaine_proxy', '0.0.0.0') : NULL);
        
        /*
         * Post Treatment
         *
         * Previous call to registerProvenanceStart() may have modified values in $dataIdIndexedArguments to transform URLs into
         * quarantained URLS. We shall iterate through it array and modify $nameIndexedArguments accordingly.
         *
         */
        if($quarantaineExecution) {
            foreach(array_keys($dataIdIndexedArguments) as $fieldDataId) {

                $fieldName = $idToNameAssociation[$fieldDataId];

                if(is_array($nameIndexedArguments[$fieldName])) {
                    $localName = $nameIndexedArguments[$fieldName];
                    $localKeys = array_keys($localName);
                    $localName[$localKeys[0]] = $dataIdIndexedArguments[$fieldDataId];
                    $nameIndexedArguments[$fieldName] = $localName;
                }
                else {
                    $nameIndexedArguments[$fieldName] = $dataIdIndexedArguments[$fieldDataId];
                }
            }
        }
        
        error_log("$algoString Clone ? " . checkForCloneExecution($runId));
        error_log("$algoString arguments: " . print_r($nameIndexedArguments,true));
        // Get the service execution results.
        $result = $client->call('callback', array('args' => new soapval('args', 'inputType_'.htmlentities($algoInfo['NAME']), $nameIndexedArguments, false/*, 'urn:convertwsdl'*/))); // call the operation of the effective service

        /** \todo check for possible execution failures of the above webservice call. */
        if ($client->fault) {
            error_log (print_r($result,true));
            registerProvenanceFailure($runId, $algoId);
            return new soap_fault('SERVER', '', 'Internal Client Fault', '');
        } else {
            // Check for errors
            $err = $client->getError();
            if ($err) {
                error_log (print_r($err,true));
                registerProvenanceFailure($runId, $algoId);
                return new soap_fault('SERVER', '', 'Internal Client Error', print_r($err,true));
            }
        }

        /*
         * Retrieve output type structure from database
         */
        $outputT = retrieveAlgorithmSOAPOutputTypes($algoId,'');

        $output = array(); /* will contain the output in nusoap compatible format and conforming to $outputT */
        $outputParameters = array(); /* will contain the output in a less constringent format - for further internal use */
        $idToNameAssociation = array(); /* an array to keep track of correspondencies between $output and $outputParameters */

        /*
         * Step 1: array_keys in the $outputT structure are supposed to be construced as <data.name>_<data.id>
         *
         * Since data records are supposed to uniquely define program arguments, an algorithm shouldn't have the
         * same kind of argument twice.
         *
         * So ... we iterate over all output arguments.
         */
        foreach(array_keys($outputT) as $fieldName) {

            /* $fieldName is of type <data.name>_<data.id> */

            /** \todo We're assuming all array keys are unique ... check if this is actually the case 100% of the time */

            $fieldValue = $outputT[$fieldName]; /* $fieldValue contains the type description of $fieldName (either as a string, either as an array) */

            /*
             * Step 2: splitting $fieldName in it's data.name and data.id part.
             *
             * $fieldDataId = data.id
             * $argumentDataId = data.name
             *
             */
            $splitFields = array();
            /** \todo Check following 2 lines of code... it looks like there's some redundancy here **/
            preg_match('/(.*)_([\d]*)$/',$fieldName,$splitFields);
            $strippedFieldName = preg_replace('/_[\d]*$/','',$splitFields[1]);
            $fieldDataId = $splitFields[2];

            $localoutput = array(); /* will contain part of the NuSoap compatible data structure */

            /*
             * Step 3: is $fieldValue is an array, then it contains comlpex type information.
             *
             * In that case the $localoutput array key should be stripped. Else just use the $fieldName as key.
             */
            if(is_array($fieldValue)) {
                /** \todo  We're assuming composite types only have one component ... check if this is actually the case 100% of the time */
                $strippedDataTypeName = preg_replace('/^tns:dae_(.*)_[\d]*$/','$1',$fieldValue['type']);
                $localoutput[$strippedDataTypeName] = $result[$strippedFieldName];
            } else {
                $localoutput[$fieldName] = $result[$strippedFieldName];
            }

            $outputParameters[$fieldDataId] = $result[$strippedFieldName];
            $output[$fieldName] = $localoutput;
            $idToNameAssociation[$fieldDataId] = $fieldName;
        }

		
        /* Register all produced output in the database for provenance */
        registerProvenanceStop($runId, $algoId, $outputParameters);

		/* 
		 * checks for hook functions for the given algorithm and calls them if it exists
		 * Hook function MUST be called after registerProvenanceStop(), since basic algorithm result data
		 * is needed by the hooks. 
		 */ 
        
		hook_algorithm($algoId,$algoString,array('id' => $runId)); // \todo place arguments in array() place holder
		
        /*
         * Post Treatment
         *
         * Previous call to registerProvenanceStop() may have modified values in $outputParameters to transform internal URLs into
         * dae-referenced URLS. We shall iterate through the $outputParameters array and modify $output accordingly.
         *
         * We deliberately use the same variable names as those used in the previous loop. This may or may not be a good idea.
         */
        foreach(array_keys($outputParameters) as $fieldDataId) {

            $fieldName = $idToNameAssociation[$fieldDataId];

            $localoutput = $output[$fieldName];
            $localKeys = array_keys($localoutput);
            $localoutput[$localKeys[0]] = $outputParameters[$fieldDataId];
            $output[$fieldName] = $localoutput;
        }

        session_destroy();
        //error_log ( date('D, d M Y H:i:s') . ' ' . print_r($output,true) . "\n", 3, '/tmp/myphplog');
        return $output;
    }
    else {
        return new soap_fault('CLIENT', '', 'Invalid or missing credentials', '');
	} 
}

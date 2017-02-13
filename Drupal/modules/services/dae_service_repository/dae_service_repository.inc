<?php

require_once("./".drupal_get_path('module','daedatabase')."/daedatabase_db.inc");

function sql_service($sql) {
    
	$err_msg=null;

	$db = getDAEDatabase();
	
	$query = $sql; 
	
    $stmt = oci_parse($db->getConn(),$query);
	
    if (!oci_execute($stmt)){
		$err_msg="cannot execute query: \"$query\"";
		$e = oci_error();    
  		databaseDebugMessage(print_r($e,true));
    }
    else {

    	$meta=array();
    	$ncols = $db->num_fields($stmt); 
    	for ($i = 0; $i < $ncols; $i++) {
    		$column_name  = $db->field_name($stmt, $i); 
    		array_push($meta,$column_name);
	    }

    	$rs=array();
    
		while ($current_row = $db->fetch_row($stmt)) { 
			$row=array();		
			for ($i = 0; $i < $ncols; $i++) {
				if ( ! is_object($current_row[$i])) {
       				array_push($row,$current_row[$i]);
				}
				else {
					/*!
				 	* \todo Using OCI_LOB->load() is a OCI specific hack, and does not use the generic dae_database API. This should be corrected in future releases
				 	*/
					try {
						array_push($row,$current_row[$i]->load());
					} catch (Exception $e) {
    					$err_msg= 'Caught exception: '.  $e->getMessage();
					}
				}
    		}
    		array_push($rs,$row);
		}
    }
		
	// 4. Cleaning up after we're done 
	oci_free_statement($stmt); // Can be generalized to mysql_fee_statement also.	
	
	if($err_msg!=null){
		return array('error_message'=>$err_msg);
	}else{
		$rslt=array('metadata'=>$meta,'result_set'=>$rs);
		return array('result_table'=>$rslt);
	}
}


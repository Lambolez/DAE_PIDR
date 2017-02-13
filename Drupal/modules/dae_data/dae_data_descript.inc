<?php

require_once(variable_get('NuSOAP Path','./'.drupal_get_path('module','soap_server')."/nusoap/lib/nusoap.php"));

function dae_data_description(){
    
    global $base_url;
    global $user;
    
    
	$msg='';
	$wsdl = "$base_url/services/soap?wsdl";
	$client = new nusoap_client($wsdl, false);
	
    $err = $client->getError();
    if ($err) {
        //error_log("ERROR dae_data_description",3,'/tmp/myphplog');
       error_log ($err);
       drupal_set_message(print_r($err,true),error);
    }
    
	DEFINE('USE_SESSION', FALSE);
	$client->debug_flag=false;
	
	$list_str = arg(2);
	$list=explode(",", $list_str);
	
	/** 
	 * sessid authentication works fine - as long as it has been activated - but interferes with other web services.
	 * 
	 * This should be reactivated after investigation and re-engineering of the whole web service workflow
	 * 
	 */
	//$result = $client->call('DAE Data.DataItem Descript', array('sessid'=> $user->sid, 'idList'=>$list));
	$result = $client->call('DAE Data.DataItem Descript', array('idList'=>$list));
	
    if ($client->fault) {
        //error_log("FAULT dae_data_description",3,'/tmp/myphplog');
        error_log (print_r($result,true));
        drupal_set_message(print_r($result,true),error);
    } else {
        $err = $client->getError();
        if ($err) {
            //error_log("ERROR2 dae_data_description",3,'/tmp/myphplog');
            error_log (print_r($err,true));
           drupal_set_message(print_r($err,true),error);
        }
    }
	
	$orig=$client->responseData;
	$after=ereg_replace("<\\?xml[\x20-\x7E]+\\?>","\\0 <?xml-stylesheet type=\"text/xsl\" href=\"$base_url/data_descript.xsl\"?>",$orig);
    
	drupal_set_header('Content-Type: text/xml; utf-8');
	print $after;
}

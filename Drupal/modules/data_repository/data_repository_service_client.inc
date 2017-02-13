<?php
function data_repository_service_client(){
	global $base_url;
    global $user;
    
    $sql=$_REQUEST['sql'];
    
	$wsdl = "$base_url/services/soap?wsdl";
	$client = new nusoap_client($wsdl, false);
	
    $err = $client->getError();
    if ($err) {
       drupal_set_message(print_r($err,true),error);
    }
    
	DEFINE('USE_SESSION', FALSE);
	$client->debug_flag=false;
	

	$result = $client->call('DAE Repository', 
		array('sql'=>$sql)
		);
	
    if ($client->fault) {
        drupal_set_message(print_r($result,true),error);
    } else {
        $err = $client->getError();
        if ($err) {
           drupal_set_message(print_r($err,true),error);
        }
    }
	
	$orig=$client->responseData;
	
	
	drupal_set_header('Content-Type: text/xml; utf-8');
	print $orig;
//	print $after;
}

function data_repository_service_client_form_page(){
    global $base_url;
    
	$output="<p>Input a SQL query here:</p><form action=\"$base_url/?q=Repository/client\" method=\"POST\">"
	."<textarea name=\"sql\" rows=\"10\" cols=\"100\"></textarea><br/><input name=\"submit\" type=\"submit\"/></form>";
	return $output;
}

<?php
// Pull in the NuSOAP code

require_once './nusoap/lib/nusoap.php';
//require_once ("./".drupal_get_path('module', 'soap_server') ."/nusoap/lib/nusoap.php");

// Create the client instance

// TODO Set the paht of wsdl
$wsdl = 'http://localhost/dae/?q=services/soap';
// N.B: allow anonymous user to access services.
/*$client = new SoapClient($wsdl, 
  array(
    'compression' => SOAP_COMPRESSION_ACCEPT | SOAP_COMPRESSION_GZIP,
  )
);*/
$client = new nusoap_client($wsdl, false);
// Set this variable TRUE if we enable use session on services settings.
DEFINE('USE_SESSION', FALSE);
$user = NULL;

$client->debug_flag=true;
//  $client->proxyhost = 'localhost';
//  $client->proxyport = 8080;
//  print_r($client);
// Call the SOAP method

// Set method parameters
/*
 * node.load params are 
 *  1- nid => int
 *  2- fields => array (optional)
 *  Let we call methos twice with & without fields 
 *  then we have $param1 & $param2
 */
 
echo 'Call dae_service <br> 
  ---------------------------------------------------------------';
  $input = array('name' => array('page_image' => 'testPara'));
  
  echo '<h3> Inputs: '; print_r("page_image: " . $input['name']['page_image']); echo '</h3>';
  
  $result = $client->call('dae.ocrad', $input); // call the proxy service wsdl
  //$result = $client->call('dae.noio'); // call the proxy service wsdl
  
  echo '<h3> Outputs: '; print_r($result); echo '</h3>';
  
	// Display the request and response
	echo '<h2>Request</h2>';

		$text = htmlspecialchars($client->request, ENT_QUOTES);
		$remove = array ("<"," x"); 
		$insert = array ("<br><","<br>    x"); 
		$text = str_replace($remove, $insert, $text);

	echo '<pre>' . $text . '</pre>';
	echo '<h2>Response</h2>';

		$text = htmlspecialchars($client->response, ENT_QUOTES);
		$remove = array ("<"," x"); 
		$insert = array ("<br><","<br>    x"); 
		$text = str_replace($remove, $insert, $text);

	echo '<pre>' . $text . '</pre>';

	// Display the debug messages
	echo '<h2>Debug</h2>';
	echo '<pre>' . htmlspecialchars($client->debug_str, ENT_QUOTES) . '</pre>';

  

echo 'Call dae_service_io <br> 
  ---------------------------------------------------------------';
  //$input = array('name' => array('page_image' => 'testPara'));
  $input = array('name' => 'testPara');
  
  echo '<h3> Inputs: '; print_r($input); echo '</h3>';
  
  $result = $client->call('dae.io', $input); // call the proxy service wsdl
  
  echo '<h3> Outputs: '; print_r($result); echo '</h3>';  

echo 'Call dae_service_o <br> 
  ---------------------------------------------------------------';
  //$input = array('name' => array('page_image' => 'testPara'));
   
  $result = $client->call('dae.o'); // call the proxy service wsdl
  
  echo '<h3> Outputs: '; print_r($result); echo '</h3>';    

echo 'Call dae_service_noio <br> 
  ---------------------------------------------------------------';
  //$input = array('name' => array('page_image' => 'testPara'));
   
  $client->call('dae.noio'); // call the proxy service wsdl
  
  //echo '<h3> Outputs: '; print_r($result); echo '</h3>';
echo '<br>';  
  
if (USE_SESSION) {
  $param['sessid'] = '1234534534dfsf';
}
$param['username'] = 'admin';
$param['password'] = 'daepassword';

echo 'Call user.login with optional params <br> 
  ---------------------------------------------------------------';
//$user = $client->__soapCall('user.login', array($param)); 
//echo $param;
$user = $client->call('user.login', $param); 
//$result = $client->call('procedure', array('name' => new soapval('name', 'complexInput', $input, false, 'urn:algolistwsdl')));
echo '<pre>';print_r($user);echo '</pre>';






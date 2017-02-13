<?php
// $Id:$

/**
 * @file
 * 
 * @author  Qihan Long <qil212@.lehigh.edu>
 * 
 * @version 0.1
 *
 * \brief The workflow module is 
 * 
 * It consists of 3 files:
 *  - workflow.info (containing the module declaration for Drupal)
 *  - workflow.module (containing a simple menu callback declaration)
 *  - workflow.inc (the actual menu callback function)
 *  - workflow_input.inc (asks the user for the input for the workflow)
 * 
 * @section LICENSE
 * 
 *  This file is part of the DAE Platform Project (DAE). 
 *  It can be downloaded from http://sourceforge.net/projects/daeplatform/
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
 *
 */
  

/**
 * Implementation of callback function.
 */

function workflow_generation() 
{   
	$output .= drupal_get_form("workflow_form");

	return $output;
}


/*
 * a form definition function that returns a file upload and submit button
 * 
 * @param [in] $form_state
 * @return a Drupal form array.
 */
function workflow_form($form_state){
	$form = array();
	
	$form['#attributes'] = array('enctype' => "multipart/form-data");
	
	$form['workflow_file'] = array(
    '#type' => 'file',
    '#title' => t('File'),
    '#description' => t('File to upload')
	);

	$form['workflow_submit'] = array(
	'#type' => 'submit',
	'#value' => t('Submit Workflow'), 
	//'#submit' => 'do_post_request'
	);

	return $form;
}

/*
 * method triggered then workflow_submit button is clicked
 */
function workflow_form_submit(){
	
	$server = "dae-orchestrator.cse.lehigh.edu:8080";// Server address
	
	
	//Opens the uploaded file and puts the data in the string $fileData
	//and the size of the file in $fileSize
	$file = file_save_upload('workflow_file');
    if (!$file) {
      form_set_error($fieldName, 'Error uploading file.');
      return;
    }
    
    //adds in the header for the file
    $fh = fopen($file->filepath, "r");
	$fileSize = filesize($file->filepath);
	$fileData = fread($fh, $fileSize);
	
	$workflowhead = "<workflow xmlns=\"http://ns.taverna.org.uk/2010/xml/server/\">";
	$workflowfoot = "</workflow>";
	$subdir = "/taverna-server/rest/runs";
	$post = createRequest("POST", $subdir, "application/xml", $workflowhead, $fileData, $workflowfoot);
	
	/* Creates a POST message with the necessary headings and sends
	 it to the Taverna server to upload the t2flow file on the server */
	$response = sendRequest($server, $post);
	$uuid = substr($response, strpos($response, "/runs")+ 6, 36);
	//echo "uuid " . $uuid;
    
    drupal_goto('workflow/input/' . $uuid);
}

/* createRequest puts together a message to be sent by concatenating the
 given method, url, header, data, and footer
 method should be POST, PUT, GET, etc.
 url is the sub directory of the server where you want to send the message*/
function createRequest($method, $url, $type, $head, $data, $foot){
	$body = $head . $data . $foot;
	$dataSize = strlen($body);

	$post = $method . " " . $url . " HTTP/1.1\r\n";
	$post .= "Host: dae-orchestrator.cse.lehigh.edu:8080\r\n";
	$post .= "Content-Length: " . $dataSize . "\r\n";
	$post .= "Content-Type: " . $type . "\r\n\r\n";
	$post .= $body;
	//echo "Done<br/>";

	return $post;
}

/* sendRequest sends the message specified in $request to the server at $url
 and returns the response from the server.*/
function sendRequest($url, $request){
	$ch = curl_init();
	curl_setopt($ch, CURLOPT_SSL_VERIFYPEER, 0);
	curl_setopt($ch, CURLOPT_URL, $url);
	curl_setopt($ch, CURLOPT_RETURNTRANSFER, 1);
	curl_setopt($ch, CURLOPT_TIMEOUT, 10);
	curl_setopt($ch, CURLOPT_CUSTOMREQUEST, $request);
	curl_setopt($ch, CURLOPT_HEADER, true);

	$data = curl_exec($ch);

	if(curl_errno($ch)){
		//TODO:print curl_error($ch);
		echo curl_error($ch) . "</br>";
	} else{
		curl_close($ch);
	}

	return $data;
}


/*
 * executes a post request, 
 * currently a place holder, hasn't been tested
 * 
 * @param $url the url to POST to
 * @param $data the data to POST
 * @param $optional_headers
 * @return a response.
 */
function do_post_request($url, $data, $optional_headers = null)
{
  $params = array('http' => array(
              'method' => 'POST',
              'content' => $data
            ));
  if ($optional_headers !== null) {
    $params['http']['header'] = $optional_headers;
  }
  $ctx = stream_context_create($params);
  $fp = @fopen($url, 'rb', false, $ctx);
  if (!$fp) {
    throw new Exception("Problem with $url, $php_errormsg");
  }
  $response = @stream_get_contents($fp);
  if ($response === false) {
    throw new Exception("Problem reading data from $url, $php_errormsg");
  }
  return $response;
}
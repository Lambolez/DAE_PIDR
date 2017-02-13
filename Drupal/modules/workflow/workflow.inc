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
	//calls the form generation function depending on the URL args.	
	if(strcmp(arg(1), "output") == 0){
		$output .= drupal_get_form("workflow_output_form");
	} else if(strcmp(arg(1), "input") == 0){
		$output .= drupal_get_form("workflow_input_form");
	} else {
		$output .= drupal_get_form("workflow_form");
	}

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
	);

	return $form;
}

/*
 * method triggered then workflow_submit button is clicked
 */
function workflow_form_submit(){
	
	$server = "http://dae-orchestrator.cse.lehigh.edu:8080";// Server address
	
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
	
	$portInfo = "/";
	
	/*Takes a workflow file located in $fileData and parses the input
	 names from the <port> tags in the workflow and puts the name in each
	 port into the array portname.
	 */
	$inputblock = stristr($fileData, "<inputports>");
	$inputendpos = strpos($inputblock, "</inputPorts>");
	$inputblock = substr($inputblock, 0, $inputendpos);
	$inputstrlen = strlen($inputblock);

	//Grabs the portion of inputblock after the first <name> tag
	$portblock = stristr($inputblock, "<name>");
	$portend = strpos($portblock, "</name>");
	
	$portnum = 0;

	//$portend will be false if there are no more </name> tags
	while($portend){
		//set up the query string
		$portInfo = $portInfo."Input";
		//Gets the information between the <name> and </name> tags
		$portInfo = $portInfo.substr($portblock, 6, $portend-6);
		$portInfo = $portInfo."/";
		//Get the next port name
		$portblock = stristr($portblock, "<port>");
		$portblock = stristr($portblock, "<name>");
		$portend = strpos($portblock, "</name>");
		$portnum += 1;
	}
	
	/*Takes a workflow file located in $fileData and parses the output
	 names from the <port> tags in the workflow and puts the name in each
	 port into the array portname.
	 */
	$outputblock = stristr($fileData, "<outputports>");
	$outputendpos = strpos($outputblock, "</outputPorts>");
	$outputblock = substr($outputblock, 0, $outputendpos);
	$outputstrlen = strlen($outputblock);

	//Grabs the portion of inputblock after the first <name> tag
	$portblock = stristr($outputblock, "<name>");
	$portend = strpos($portblock, "</name>");

	$portnum = 0;
	//$portend will be false if there are no more </name> tags
	while($portend){
		//set up the query string
		$portInfo = $portInfo."Output";
		//Gets the information between the <name> and </name> tags
		$portInfo = $portInfo.substr($portblock, 6, $portend-6);
		$portInfo = $portInfo."/";
		//Get the next port name
		$portblock = stristr($portblock, "<port>");
		$portblock = stristr($portblock, "<name>");
		$portend = strpos($portblock, "</name>");
		$portnum += 1;
	}
		
    drupal_goto('workflow/input/' . $uuid . $portInfo);
}

/*
 * a form definition function that returns a file upload and submit button
 * 
 * @param [in] $form_state
 * @return a Drupal form array.
 */
function workflow_output_form($form_state){
	$uuid = arg(2);
	
	$server = "http://dae-orchestrator.cse.lehigh.edu:8080";// Server address
	
	$argNum = 3;
	$arg = arg($argNum);

	//Skip past the input arguements
	while(strcasecmp(substr($arg, 0, 5), "Input") == 0 && $arg != NULL){
		$argNum++;
		$arg = arg($argNum);
	}
	
	//creates request to check if workflow is complete
	$subdir = "/taverna-server/rest/runs/" . $uuid . "/status";
	$statusmsg = createGetRequest($subdir);
	$response = sendRequest($server, $statusmsg);
	
	//waits for workflow to complete
	while(!stristr($response,"Finished")){
		$response = sendRequest($server, $statusmsg);
	}
	
	$form = array();
	
	$form['#attributes'] = array('enctype' => "multipart/form-data");	

	//Sends messages to Taverna to get the result for each output port
	while(strcasecmp(substr($arg, 0, 6), "Output") == 0 && $arg != NULL){
		$portName = substr($arg, 6);
		
		$subdir = "/taverna-server/rest/runs/" . $uuid . "/wd/out/".$portName;
		$outputmsg = createGetRequest($subdir);
		$response = sendRequest($server, $outputmsg);
		$response = remove_header($response);
		$response = parse_output($response);
		$response = "<h3>".$portName."</h3><p>".$response."</p></br>";


		$form['workflow_result_'.$portName] = array(
    '#type' => 'markup',
    '#value' => $response
		);
		
		
		$argNum++;
		$arg = arg($argNum);
	}
	
		$form['workflow_output_submit'] = array(
	'#type' => 'submit',
	'#value' => t('New Workflow'), 
	);

	return $form;
	
}

//return to workflow page to submit a new workflow
function workflow_output_form_submit($form_state){
	drupal_goto('workflow/');
}

/*
 * a form definition function that returns a textfield for the input and submit button
 * 
 * @param [in] $form_state
 * @return a Drupal form array.
 */
function workflow_input_form($form_state){
	$form = array();
	
	$form['#attributes'] = array('enctype' => "multipart/form-data");
	
	$argNum = 3;
	$arg = arg($argNum);
	
	//Creates an input field for each input specified in the URL
	while(strcasecmp(substr($arg, 0, 5), "Input") == 0 && $arg != NULL){
		$portName = substr($arg, 5);
		$form['workflow_input_'.$portName] = array(
    '#type' => 'textfield',
    '#title' => t($portName),
		);
		$argNum++;
		$arg = arg($argNum);
	}

	$form['workflow_input_submit'] = array(
	'#type' => 'submit',
	'#value' => t('Submit Input'), 
	);
	return $form;
}

/*
 * method triggered then workflow_input_form_submit button is clicked
 * 
 * Sends in the inputs and tells Taverna to start running
 * then redirects to output page
 */
function workflow_input_form_submit($form_state){
	$uuid = arg(2);
	
	$server = "http://dae-orchestrator.cse.lehigh.edu:8080";// Server address
	$inputhead = "<t2sr:runInput xmlns:t2sr=\"http://ns.taverna.org.uk/2010/xml/server/rest/\">\r\n";
	$inputfoot = "</t2sr:runInput>";
	
	$argNum = 3;
	$arg = arg($argNum);
	
	//Sends a PUT message to Taverna for each input port
	while(strcasecmp(substr($arg, 0, 5), "Input") == 0 && $arg != NULL){
		$portName = substr($arg, 5);
		$input = $_POST['workflow_input_'.$portName];

		$inputData = "<t2sr:value>" . $input . "</t2sr:value>\r\n";
		$subdir = "/taverna-server/rest/runs/" . $uuid . "/input/input/" . $portName;
		$inputpost = createRequest("PUT", $subdir, "application/xml",
		$inputhead, $inputData, $inputfoot);
		$response = sendRequest($server, $inputpost);
		
		$argNum++;
		$arg = arg($argNum);
	}

	//Sends a PUT message with the command Operating to run the workflow with the given input
	$subdir = "/taverna-server/rest/runs/" . $uuid . "/status";
	$runpost = createRequest("PUT", $subdir, "text/plain", "Operating", "", "");
	sendRequest($server, $runpost);
	
	
	$argNum = 3;
	$arg = arg($argNum);
	
	while($arg != NULL){
		$uuid = $uuid . "/" . arg($argNum);
		$argNum++;
		$arg = arg($argNum);
	}
	
	drupal_goto('workflow/output/' . $uuid);
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
	
	$error = curl_errno($ch);

	if(curl_errno($ch)){;
		
		drupal_set_message("Connecting to $url: ".curl_error($ch),error);
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

/* createGetRequest simply sends a GET message to the given url */
function createGetRequest($url){
	$post = "GET " . $url . " HTTP/1.1\r\n";
	$post .= "Host: dae-orchestrator.cse.lehigh.edu:8080\r\n";
	$post .= "Accept: */*\r\n\r\n";
	
	return $post;
}

//removes the header from the workflow results
function remove_header($text){
	$content_length_start = strpos($text, "Content-Length:");
	$text = substr($text, $content_length_start + 10);
	$space = strpos($text, " ");
	$text = substr($text, $space);
	$space = strpos($text, "
");
	$text = substr($text, $space);
	$text = trim($text);
	return $text;
}

//parses the xml workflow results into a displayable format
function parse_output($text){
	$tagStart = strpos($text, "<");
	if($tagStart === FALSE){
		$end = substr($text, strlen($text) - 4);
		if(strcmp($end, ".txt") == 0){
			$final = "<a href=\"".$text."\">".$text."</a>";
			return $final;
		}
		return $text;
	}
	$tagEnd = strpos($text, ">");
	$tag = substr($text, $tagStart + 1, $tagEnd - $tagStart - 1);
	$rest = substr($text, $tagEnd + 1);
	if(strcmp(substr($tag, strlen($tag) - 1), "/") == 0){
		$final = substr($tag, 0, strlen($tag) - 2).":";
		if(strlen($rest) > 0){
			$final = $final."</br>".parse_output($rest);
		}
		return $final;
	}
	$tagSpace = strpos($tag, " ");
	$tagNoSpace;
	if(!$tagSpace){
		$tagNoSpace = $tag;
	} else {
		$tagNoSpace = substr($tag, 0, $tagSpace);
	}
	$endTag = "</".$tagNoSpace.">";
	$betweenTags = substr($text, $tagEnd + 1, strpos($text, $endTag) - $tagEnd - 1);
	$afterTags = substr($text, strpos($text, $endTag) + strlen($endTag));
	$final = $tag.":";

	if(!strpos($betweenTags, ">")){
		$final = $final." ".$betweenTags."</br>";
	} else {		
		$final = $final."</br>";
		$final = $final.parse_output($betweenTags);
	}
	
	if(strlen($afterTags) > 0){
		$final = $final.parse_output($afterTags);
	}
	return $final;
}
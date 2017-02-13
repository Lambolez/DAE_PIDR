<?php
/**
 * @file client.php
 * 
 * @author  Yingjie Li <yil308@lehigh.edu>
 * @author  Bart Lamiroy <lamiroy@cse.lehigh.edu>
 * 
 * @version 1.0
 *
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

require_once('lib/nusoap.php');
include('setup.php');

// Create the client instance
$client = new nusoap_client($WSDLURL.'?wsdl', true);

// Retrieve debug information.
$err = $client->getError();
if ($err) {
    // Display the error
    echo '<h2>Constructor error</h2><pre>' . $err . '</pre>';
    // At this point, you know the call that follows will fail
}

//== START EDITABLE ZONE
$input = array('page_image' => 'http://static.php.net/www.php.net/images/php.gif');
//== STOP EDITABLE ZONE

//Invoke the web service SOAP method.
$result = $client->call('callback', array('args' => $input));

/*
 * Instead of the previous code something in the lines of 
 * 
 * $result = $proxy->procedure_ocrad('http://static.php.net/www.php.net/images/php.gif'); Does NOT work.
 * 
 * or perhaps
 * 
 * $result = $proxy->procedure_ocrad(array('name' => array('page_image' => 'http://static.php.net/www.php.net/images/php.gif'))); It works.
 * 
 * should work as well, according to the documentation. To check !!
 * 
 */


// Check for a fault
if ($client->fault) {
    echo '<h2>Fault</h2><pre>';
    print_r($result);
    echo '</pre>';
} else {
    // Check for errors
    $err = $client->getError();
    if ($err) {
        // Display the error
        echo '<h2>Error</h2><pre>' . $err . '</pre>';
    } else {
        // Display the result
        echo '<h2>Result</h2><pre>';
        print_r($result);
    echo '</pre>';
    }
}
// Display the request and response
echo '<h2>Request</h2>';
echo '<pre>' . htmlspecialchars($client->request, ENT_QUOTES) . '</pre>';
echo '<h2>Response</h2>';
echo '<pre>' . htmlspecialchars($client->response, ENT_QUOTES) . '</pre>';
// Display the debug messages
echo '<h2>Debug</h2>';
echo '<pre>' . htmlspecialchars($client->debug_str, ENT_QUOTES) . '</pre>';
?>

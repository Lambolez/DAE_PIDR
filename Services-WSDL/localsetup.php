<?php 

/**
 * @file setup.php
 * 
 * @author  Bart Lamiroy <lamiroy@cse.lehigh.edu>
 * 
 * @date October 2010
 *  
 * @version 1.0
 * 
 * \brief  Global variable settings for demo purposes.
 * 
 * \section DAEParams DAE Related Parameters
 * 
 *  DAE Related Parameters are those that allow to relate the provided service to
 *  a DAE registered algorithm. The are:
 *   - $algoOracleID
 *   - $algoname
 *   - $inputT
 *   - $outputT
 *   - $WSDLURL
 * 
 *  Values of these variables should be defined with extreme caution since they can
 *  impact global performances of the DAE process chain. 
 * 
 * \section LocalParams Local Webserver Parameters
 * 
 *  Local Webserver Parameters are:
 *   - $htmlBaseDir
 *   - $executionDir
 *   - $executionPrefix
 * 
 */

/**
 * Base directory for local webserver data. This directory is needed for storing 
 * final results of an execution and in order to allow results to be downloaded.
 *  
 * @var $htmlBaseDir
 */
$htmlBaseDir = '/var/www/';

/**
 * Subdirectory name (relative to \a $htmlBaseDir) where final execution results will be
 * stored. Each individual execution will create a unique subdirectory under \a $executionDir.
 * 
 * @var $executionDir
 */
$executionDir = 'wsdl';

/**
 * Filename prefix to be used for storing intermediate results. Can be left blank. 
 * 
 * @var $executionPrefix
 */
$executionPrefix = 'wsdl_';

/**
 * \brief URL that will host the provided service.
 *  Hasan: old value -> .cse.lehigh.edu
 * @var $WSDLURL
 */

$hostname = 'localhost';
$WSDLURL = "http://$hostname/$executionDir/$algoname.php";

/**
 * USE CAUTION ! Possible password exposure. 
 * 
 *  By setting this variable you will be able to download data from the DAE server 
 *  
 *  The source code transmitting your username/password does not verify whether it is actually
 *  transmitting them
 * 
 * @var $DAEUsername
 * @var $DAEPassword
 */
$DAEUsername = 'your-username';
$DAEPassword = 'your-password';

/* ============================================================================== */

/* 
 * NO EDITING REQUIRED BEYOND THIS ZONE !
 * 
 */

/* 
*/

/**
 * 
 * This function connects to the URL using a POST command 
 * and retrieves only the returned header information.
 * 
 * In the event of the URL server name to be dae.cse.lehigh.edu
 * the function will submit connection information based on the
 * global defined $DAEUsername $DAEPassword.
 * 
 * @param [in] $url the URL from which to retrieve header information
 * 
 * @return a string containing all HTTP header information.
 * 
 * This code comes from and has been adapted from
 * http://forums.digitalpoint.com/showthread.php?t=1029916#post9241653
 * 
 */
function HTTP_Get_Header( $url ) {
    
    /*
     * These variables were declared in the general variable declaration
     * section of this file.
     */
    global $DAEUsername;
    global $DAEPassword;
    
    $get_timeout   = 40;
    $s_Complete    = parse_url($url);
    if( !isset($s_Complete["scheme"] ) ) {
      $s_Complete["host"] = $s_Complete["path"];
      $s_Complete["path"] = '';
    }
    $s_Host        = $s_Complete["host"];
    if( @$s_Complete["path"] == "" )
      $s_Complete["path"] = "/?";
    $s_URI         = $s_Complete["path"];
    if( @$s_Complete["query"] != "" )
      $s_URI      .= '?'.$s_Complete['query'];
    if( @$s_Complete["port"] != "" )
      $s_Port      = $s_Complete["port"];
    else
      $s_Port      = 80;
      
    if(!strcmp('dae.cse.lehigh.edu',$s_Host)) {
        $requestMsg = 'name='.urlencode($DAEUsername).'&pass='.urlencode($DAEPassword).'&op=Log%20in&form_id=user_login_block';
    }
    else { 
        $requestMsg ='';
    }
    $requestLength = strlen($requestMsg);
    
    $request       = "POST $s_URI HTTP/1.0\r\n";
    $request      .= "Accept: */*\r\n";
    $request      .= "Host: $s_Host\r\n";
    $request      .= "Connection: Close\r\n";
    $request      .= "Content-Type: application/x-www-form-urlencoded\r\n";
    $request      .= "Content-Length: $requestLength\r\n";
    $request      .= "\r\n";
    $request	  .= $requestMsg;
    $fp = @fsockopen( $s_Host, $s_Port, $errno, $errstr, 30 );

    if( is_resource($fp) ) {
        fputs( $fp, $request );
        $query_timeout = 30;
        stream_set_blocking( $fp, true );
        stream_set_timeout( $fp, $query_timeout );
        $loop_time = time();
        $status = socket_get_status( $fp );
        $line = "";
        $header = "";

        while( !($line == "\r\n")  && !feof($fp) && !$status['timed_out'] ) {
            $line = fgets($fp, 4096);
            $header .= $line;
            $diff = time() - $loop_time;

            if( $diff > $get_timeout )
            break;

            if( connection_aborted() )
            break;

            $status = socket_get_status( $fp );
        }

        fclose( $fp );
        return $header;
    }

    return false;
}

/**
 *
 */
function DAEAuthCopy($source, $dest) {

    $header = HTTP_Get_Header($source);
        
    if($header) {
        $beginPos = strrpos($header,'Set-Cookie');
        $header = substr($header, $beginPos+4);
        $endPos = strpos($header,';');
        $header = substr($header, 0, $endPos);

        if($header) {
            $opts = array(
  				'http'=>array(
    			'method'=>"GET",
    			'header'=>"Accept: */*\r\n" .
	          	  	  "Connection: Keep-alive\r\n" .
	          	  	  "$header\r\n"            
                  )
             );
            $context = stream_context_create($opts);
            $fp = fopen($source,'r',false,$context);
        } else {
            $fp = fopen($source,'r');
        }

        $fpout = fopen($dest,'w');
    
        if(!$fp || !$fpout ) return false;
    
        while (!feof ($fp)) {
               $line = fread ($fp, 4092);
               fwrite($fpout,$line);
        } 

        fclose($fpout);
        fclose($fp);
    
        return true;
    }
    else {
        return copy($source,$dest);
    }
}

?>
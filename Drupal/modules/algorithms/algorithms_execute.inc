<?php 

/**
 * @file algorithms_execute.inc 
 * 
 * @author  Mike Kot <mdk312@lehigh.edu>
 * @author  Bart Lamiroy <lamiroy@cse.lehigh.edu>
 * 
 * @version 1.0
 * @date Last Updated 9/18/10
 *
 * \brief For the purposes of keeping hardcoded algorithm execution separate, all hardcoded run functions are in this file.
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
 *
 *
 * @section WRITING YOUR METHOD
 * 
 *  To add a procedure for a given algorithm, simply declare a function with 
 *  the name "run_\<algorithm id\>($input)" in a similiar fashion to the ones 
 *  below such that \<algorithm id\> is the id of the algorithm in the oracle database.
 *  
 * 	$input is an array of the format $input["file_\<input_number\>"] = '\<filepath\>' 
 *  where the file path is relative to the users uploads directory.  Begin your 
 *  function by declaring global $uploads; and then you can refer to files by 
 *  concatenating $uploads and a filepath. $spool and $uploads have no trailing slashes, 
 *  and filepaths begin with /, so there is no need to concatenate a slash when creating 
 *  a full path (ex: $uploads . $filepath will produce a correct path)
 *  
 *  $input also contains the run id of the algorithm run: $input['run_id']
 *  
 *  As of 5/8/10 querying the database for an algorithm's required filetypes is not 
 *  implemented, so for now there will be only one input ($input["file_0"])
 *  
 *  Note: if you use a variable in place of the 0 because you have more than one 
 *  file input (input["file_$i"]) you must put the array key in double quotes or the 
 *  string $i will be passed instead of the value of $i.
 *  
 *  Printing or echoing any text will cancel the construction of the web page, 
 *  so simply take care of whatever system calls are necessary.  No return value is necessary.
 *  
 *  User id can be accessed by adding global $user; and referring to $user->uid
 *  
 *  Look at run_XX for some example code.
 *  
 * @section REQUIREMENTS
 * 
 *  In order for the webserver to retreive the results correctly, please follow 
 *  these guidelines:
 *    - Run the method algorithms_create_spool_directories($input) and pass the $input.  
 *      This call accomplishes the following:
 *        - Creates a subdirectory of $spool (/dae/spool/\<user_id\>/\<algorithm_run_id\>/) 
 *          named after the algorithm run id ($input['run_id']) resulting a directory 
 *          structure like /dae/spool/12/450 where 12 is a user id and 450 is the 
 *          algorithm run id.  Note: $spool = /dae/spool/\<user_id\>
 *        - Creates an output directory (/dae/spool/\<user_id\>/\<algorithm_run_id\>/output)
 *        - Returns path to /dae/spool/\<user_id\>/\<algorithm_run_id\> where input belongs 
 *		    and output directory is
 *    - Copy all input files to $spool . "/" . $input['run_id'] (/dae/spool/\<user_id\>/\<algorithm_run_id\>/)
 *    - Declare the output directory as $spool . "/" . $input['run_id']. "/output"
 *    - Make sure your system calls/helper script will result in an end time being placed 
 *      in the database when the run completes
 *    - For debugging purposes, the website does not forward to the runs page so that you 
 *      can echo variables for testing purposes.  Finished implementations should not have 
 *      any echos or prints.
 */


global $db;
$db = getDAEDatabase();
 
/**
 * \brief Executes an algorithm identified by its database id and its input parameter string
 * 
 * @param $input algorithm input array
 * @param $onDatabase database instance
 * @return true if execution successfully launched, false otherwise
 */
function algorithm_execute($input,$onDatabase)
{
	global $spool;
	
	$call = "run_" . $input['id'];
	$db = getDAEDatabase();
		
	if(!function_exists($call))
	{
		databaseDebugMessage("Error: Callback function not implemented for " . $db->r("select NAME from ALGORITHM where ID = " .$input['id']) . " (".$input['id'].")");
		return 1;
	}
	else 
	{
		if(!is_dir($spool))
			mkdir($spool);
	
		$input['call_path'] = $db->r("select PATH from ALGORITHM where ID = " .$input['id']);
		$input['on_db'] = $onDatabase;
		return($call($input));
	}
}

/**
 * \brief Creates a file in the \a $runFolder containing all dataitems contained in \a $input 
 * 
 * @param [in] $runFolder
 * @param [in] $input
 */
function create_database_flag($runFolder,$input)
{
	$myFile = $runFolder . "/database.txt";
	$fh = fopen($myFile, 'w');
	
	foreach($input as $dataitem):
		$output .= $dataitem."\n";
	endforeach;
	
	fwrite($fh, $output);
	fclose($fh);
}
/**
 * This function requires the global variable \a $spool to be set.
 * It will create a subdirectory under the \a $spool directory identified by the
 * current \a run identifier. It also creates a subdirectory 'output' under the previously mentioned one,
 *  
 * @param $input
 * @return the name (string) of the created directory under \a $spool.
 */
function algorithms_create_spool_directories($input)
{
	global $spool;
	
	$runDir = $spool . '/' . $input['run_id'];

	mkdir($runDir); //Seemed to give web sandbox trouble, switched back until bug worked out
	
	$outputDir = $runDir. '/output';
	
	mkdir($outputDir); 	
	//mkdir($outputDir,0700,true);  //Seemed to give web sandbox trouble, switched back until bug worked out

	return $runDir;
}
function algorithms_get_file_name($path,$ext = true)
{	
	if($ext)
		return '/'. basename($path);
	else
	{
		$name = basename($path);
		$split = explode(".",$name);
		unset($split[count($split)-1]);
		return "/".implode(".",$split);
	}
}
function algorithms_get_extension_name($path)
{
	$file = explode(".",$path); 
	end($file);
	
	return $file[key($file)];

}
//----------------------------------------------------------------------------------------
/* 
 * This is a hack for compatibility with the standalone MySQL database.
 * Since algorithm IDs are different, the run_XX hack doesn't work.
 * 
 *  Redefining new run_XX functions to conform to MySQL numbering.
 */
function run_2($input) 
{
	return(run_82($input));
}

/**
 * \brief convert
 * 
 * @param $input
 */
function run_102($input) 
{
	global $uploads, $user, $spool, $db;

		
	$userID = $user->uid;
	$userEmail = $user->mail;
	$runID = $input['run_id']; 

	$runFolder = algorithms_create_spool_directories($input); //creates the proper directories in /dae/spool/<user_id>
	$outputFolder = $runFolder . "/output"; //standard output directory is always run location /output
	
	if($input['on_db'])
	{
		//create_database_flag($runFolder,array($input['id']));
		$inputFilePath = $db->r("SELECT PATH FROM PAGE_IMAGE WHERE ID = ".$input['file_0']);
	}
	else
		$inputFilePath = $uploads.$input['file_0']; //creates path to file in /dae/userdata/<user_id>/uploads/<path_to_file>
	
	$file = algorithms_get_file_name($inputFilePath); //strips the names of any directories from the file path relative to /dae/userdata/<user_id>/uploads/
	
	$spoolInput = $runFolder . $file; //name of the destination for our copy without leaving something like /mydir/image.png	
	$spoolOutput = $outputFolder . algorithms_get_file_name($inputFilePath,false) . '.'  . $input['file_1']; //name of the destination for our copy without leaving something like /mydir/image.png	
	
	copy($inputFilePath,$spoolInput); //copy file from user directory to spool for running

	drupal_set_message("This current implementation uses fixed parameters and does currently not allow others",warning);
	
	$convertParameters = "$spoolInput $spoolOutput";
	
	$fullPathToProgram = $input['call_path'] . ' ' . $convertParameters;
	
	watchdog('run debug',t('!c',array('!c' => $fullPathToProgram)));	
	/**
	 * \todo These values should be global variables
	 */
	$processUser = posix_getpwuid(posix_geteuid());
	$processUsername = $processUser['name'];
	
	/*!
	 * \todo Solve a major security hole !
	 * \bug This is a major security hole. People can set any value in 'algorithms_app_server' and just pipe comments to ssh !!
	 * 
	 * This is just a minor problem currently, since this code is not distributed and only people having admin access rights can modify
	 * the algorithms_app_server value.
	 * 
	 */
	$command = "ssh-agent  ssh-add " . variable_get('algorithms_ssh_id_file', '~/.ssh/id_rsa') . " && " 
	. "ssh -l '" . variable_get('algorithms_ssh_username', $processUsername) . "' " . variable_get('algorithms_app_server', '') . " 'nohup scripts/DAE_helper.sh "
	. $userID
	. " " 
	. $userEmail
	. " "
	. $runID
	. " "
	. variable_get('dae_db_name','daetoy')
	. " "
	. $fullPathToProgram
	. " > log/nohup." . $input['run_id'] . "." . $user->mail . " 2>&1 &' "
	;

	databaseDebugMessage($command); //until we get this working.
	
	system($command, $returnValue);
	return($returnValue);
}

/**
 * \brief Ocrad OCR
 * 
 * @param $input
 */
function run_143($input) 
{
	global $uploads, $user, $spool, $db;
	
	$userID = $user->uid;
	$userEmail = $user->mail;
	$runID = $input['run_id']; 

	$runFolder = algorithms_create_spool_directories($input); //creates the proper directories in /dae/spool/<user_id>
	$outputFolder = $runFolder . "/output"; //standard output directory is always run location /output
	
	/**
	 * This is where the hacking starts. We're assuming (since we know it) that there's only one file input parameter (argument 2) and a file extension (argument 1)
	 */
	
	// What follows really is a lousy hack. We known that only one of the 3 possible file_X exists.
	$fileField = $input['file_0'].$input['file_1'].$input['file_2'];
	$inputFilePath = $uploads.$fileField; //creates path to file in /dae/userdata/<user_id>/uploads/<path_to_file>
	$file = algorithms_get_file_name($fileField); //strips the names of any directories from the file path relative to /dae/userdata/<user_id>/uploads/
	
	$spoolInput = $runFolder . $file; //name of the destination for our copy without leaving something like /mydir/image.png	
	$spoolOutput = $outputFolder . $file; //name of the destination for our copy without leaving something like /mydir/image.png	

	if(!copy($inputFilePath,$spoolInput)) //copy file from user directory to spool for running
	{	
		return 1;
	}
	
	drupal_set_message("This current implementation uses fixed parameters and does currently not allow others",warning);
	
	$ocradParameters = "$spoolInput -o $spoolOutput.txt -l2 -x $spoolOutput.ocr ";
	
	$fullPathToProgram = $input['call_path'] . ' ' . $ocradParameters;
		
	/**
	 * \todo These values should be global variables
	 */
	$processUser = posix_getpwuid(posix_geteuid());
	$processUsername = $processUser['name'];
	
	/*!
	 * \todo Solve a major security hole !
	 * \bug This is a major security hole. People can set any value in 'algorithms_app_server' and just pipe comments to ssh !!
	 * 
	 * This is just a minor problem currently, since this code is not distributed and only people having admin access rights can modify
	 * the algorithms_app_server value.
	 * 
	 */
	$command = "ssh-agent  ssh-add " . variable_get('algorithms_ssh_id_file', '~/.ssh/id_rsa') . " && " 
	. "ssh -l '" . variable_get('algorithms_ssh_username', $processUsername) . "' " . variable_get('algorithms_app_server', '') . " 'nohup scripts/DAE_helper.sh "
	. $userID
	. " " 
	. $userEmail
	. " "
	. $runID
	. " "
	. variable_get('dae_db_name','daetoy')
	. " "
	. $fullPathToProgram
	. " > log/nohup." . $input['run_id'] . "." . $user->mail . " 2>&1 &' "
	;

	databaseDebugMessage($command); //until we get this working.
	
	system($command, $returnValue);
	return($returnValue);
}

/**
 * \brief Tesseract OCR
 * 
 * @param $input
 */
function run_125($input) 
{
	global $uploads, $user, $spool, $db;
	
	$userID = $user->uid;
	$userEmail = $user->mail;
	$runID = $input['run_id']; 

	$runFolder = algorithms_create_spool_directories($input); //creates the proper directories in /dae/spool/<user_id>
	$outputFolder = $runFolder . "/output"; //standard output directory is always run location /output
	
	/**
	 * This is where the hacking starts. We're assuming (since we know it) that there's only one file input parameter
	 */
	
	if($input['on_db'])
	{
		$inputFilePath = $db->r("SELECT PATH FROM PAGE_IMAGE WHERE ID = ".$input['file_0']);
	}
	else
		$inputFilePath = $uploads.$input['file_0']; //creates path to file in /dae/userdata/<user_id>/uploads/<path_to_file>
	

	$file = algorithms_get_file_name($inputFilePath); //strips the names of any directories from the file path relative to /dae/userdata/<user_id>/uploads/
	
	$spoolInput = $runFolder . $file; //name of the destination for our copy without leaving something like /mydir/image.png	
	$spoolOutput = $outputFolder . $file; //name of the destination for our copy without leaving something like /mydir/image.png	
	
	copy($inputFilePath,$spoolInput); //copy file from user directory to spool for running

	drupal_set_message("This current implementation uses fixed parameters and does currently not allow others",warning);
	
	$tesseractParameters = "$spoolInput $spoolOutput";
	//$fullPathToFile = $uploads.$input["file_0"];
	
	$fullPathToProgram = $input['call_path'] . ' ' . $tesseractParameters;
		
	/**
	 * \todo These values should be global variables
	 */
	$processUser = posix_getpwuid(posix_geteuid());
	$processUsername = $processUser['name'];
	
	/*!
	 * \todo Solve a major security hole !
	 * \bug This is a major security hole. People can set any value in 'algorithms_app_server' and just pipe comments to ssh !!
	 * 
	 * This is just a minor problem currently, since this code is not distributed and only people having admin access rights can modify
	 * the algorithms_app_server value.
	 * 
	 */
	$command = "ssh-agent  ssh-add " . variable_get('algorithms_ssh_id_file', '~/.ssh/id_rsa') . " && " 
	. "ssh -l '" . variable_get('algorithms_ssh_username', $processUsername) . "' " . variable_get('algorithms_app_server', '') . " 'nohup scripts/DAE_helper.sh "
	. $userID
	. " " 
	. $userEmail
	. " "
	. $runID
	. " "
	. variable_get('dae_db_name','daetoy')
	. " "
	. $fullPathToProgram
	. " > log/nohup." . $input['run_id'] . "." . $user->mail . " 2>&1 &' "
	;

	databaseDebugMessage($command); //until we get this working.
	
	system($command, $returnValue);
	return($returnValue);
}

/**
 * \brief Kanungo Degradation Program (from QGar Library)
 * 
 * @param $input
 */
function run_124($input) 
{
	global $uploads, $user, $spool, $db;
	
	$userID = $user->uid;
	$userEmail = $user->mail;
	$runID = $input['run_id']; 

	$runFolder = algorithms_create_spool_directories($input); //creates the proper directories in /dae/spool/<user_id>
	$outputFolder = $runFolder . "/output"; //standard output directory is always run location /output
	
	/**
	 * This is where the hacking starts. We're assuming (since we know it) that there's only one file input parameter
	 */
	
	if($input['on_db'])
	{
		$inputFilePath = $db->r("SELECT PATH FROM PAGE_IMAGE WHERE ID = ".$input['file_0']);
	}
	else
		$inputFilePath = $uploads.$input['file_0']; //creates path to file in /dae/userdata/<user_id>/uploads/<path_to_file>

	$file = algorithms_get_file_name($inputFilePath); //strips the names of any directories from the file path relative to /dae/userdata/<user_id>/uploads/
	
	$spoolInput = $runFolder . $file; //name of the destination for our copy without leaving something like /mydir/image.png	
	$spoolOutput = $outputFolder . $file; //name of the destination for our copy without leaving something like /mydir/image.png	
	
	copy($inputFilePath,$spoolInput); //copy file from user directory to spool for running

	drupal_set_message("This current implementation uses fixed parameters: -a0 0.5 -a 0.6 -b0 0.4 -b 0.6 -eta 0.1 -size 1",warning);
	
	$kanungoParameters = "-in $spoolInput -out $spoolOutput -a0 0.5 -a 0.6 -b0 0.4 -b 0.6 -eta 0.1 -size 1";
	$fullPathToFile = $uploads.$input["file_0"];
	
	$fullPathToProgram = $input['call_path'] . ' ' . $kanungoParameters;
		
	/**
	 * \todo These values should be global variables
	 */
	$processUser = posix_getpwuid(posix_geteuid());
	$processUsername = $processUser['name'];
	
	/*!
	 * \todo Solve a major security hole !
	 * \bug This is a major security hole. People can set any value in 'algorithms_app_server' and just pipe comments to ssh !!
	 * 
	 * This is just a minor problem currently, since this code is not distributed and only people having admin access rights can modify
	 * the algorithms_app_server value.
	 * 
	 */
	$command = "ssh-agent  ssh-add " . variable_get('algorithms_ssh_id_file', '~/.ssh/id_rsa') . " && " 
	. "ssh -l '" . variable_get('algorithms_ssh_username', $processUsername) . "' " . variable_get('algorithms_app_server', '') . " 'nohup scripts/DAE_helper.sh "
	. $userID
	. " " 
	. $userEmail
	. " "
	. $runID
	. " "
	. variable_get('dae_db_name','daetoy')
	. " "
	. $fullPathToProgram
	. " > log/nohup." . $input['run_id'] . "." . $user->mail . " 2>&1 &' "
	;

	databaseDebugMessage($command); //until we get this working.
	
	system($command, $returnValue);
	return($returnValue);
}

//-----------------------------------------------------------------------------------------

function run_82 ($input) // DICE
{
	global $uploads, $user, $spool, $db;
	
	$runFolder = algorithms_create_spool_directories($input); //creates the proper directories in /dae/spool/<user_id>
	$outputDir = $runFolder . "/output"; //standard output directory is always run location /output
	
	//databaseDebugMessage(print_r($input));
	//databaseDebugMessage($runFolder);
	
	/**
	 * This is where the hacking starts. We're assuming (since we know it) that there's only one file input parameter
	 */
	
	if($input['on_db'])
	{
		$inputFilePath = $db->r("SELECT PATH FROM PAGE_IMAGE WHERE ID = ".$input['file_0']);
	}
	else
		$inputFilePath = $uploads.$input['file_0']; //creates path to file in /dae/userdata/<user_id>/uploads/<path_to_file>

	$file = algorithms_get_file_name($inputFilePath); //strips the names of any directories from the file path relative to /dae/userdata/<user_id>/uploads/
	
	
	$spoolInput = $runFolder . $file; //name of the destination for our copy without leaving something like /mydir/image.png	
	copy($inputFilePath,$spoolInput); //copy file from user directory to spool for running

	/**
	 * \todo These values should be global variables
	 */
	$processUser = posix_getpwuid(posix_geteuid());
	$processUsername = $processUser['name'];
	
	/*!
	 * \todo Solve a major security hole !
	 * \bug This is a major security hole. People can set any value in 'algorithms_app_server' and just pipe comments to ssh !!
	 * 
	 * This is just a minor problem currently, since this code is not distributed and only people having admin access rights can modify
	 * the algorithms_app_server value.
	 * 
	 */
	 
	$fullPathToProgram = $input['call_path'] . " " . $spoolInput . " " . $outputDir;
	 
	/*$command = "ssh-agent ssh-add " . variable_get('algorithms_ssh_id_file', '~/.ssh/id_rsa') . " && " 
	. "ssh -l '" . variable_get('algorithms_ssh_username', $processUsername) . "' " . variable_get('algorithms_app_server', '')
	. " 'nohup " . $path . " "
	. $spoolInput
	. " " 
	. $outputDir
	. " "
	. $input['run_id']
	. " "
	. variable_get('dae_db_name','daetoy')
	. " "
	. $user->mail
	. " > log/nohup." . $input['run_id'] . "." . $user->mail . " 2>&1 &' "
	;
	*/
	$command = "ssh-agent  ssh-add " . variable_get('algorithms_ssh_id_file', '~/.ssh/id_rsa') . " && " 
	. "ssh -l '" . variable_get('algorithms_ssh_username', $processUsername) . "' " . variable_get('algorithms_app_server', '') . " 'nohup scripts/DAE_helper.sh "
	. $userID
	. " " 
	. $userEmail
	. " "
	. $runID
	. " "
	. variable_get('dae_db_name','daetoy')
	. " "
	. $fullPathToProgram
	. " > log/nohup." . $input['run_id'] . "." . $user->mail . " 2>&1 &' "
	;

	databaseDebugMessage($command); //until we get this working.
	
	system($command, $returnValue);
	
	//databaseDebugMessage("Return value was = " . $returnValue); //until we get this working.
	return($returnValue);
}

function run_XX($path,$input) //just example code/usage for a made up algorithm with id XX
{
	global $uploads, $user, $spool;
	
	$userID = $user->uid;
	$userEmail = $user->mail;
	$runID = $input['run_id']; 

	$runFolder = algorithms_create_spool_directories($input); //creates the proper directories in /dae/spool/<user_id>
	$outputDir = $runFolder . "/output"; //standard output directory is always run location /output
	
	
	$fullPathToProgram = $path;
	$fullPathToFile = $uploads.$input["file_0"];
	
	/**
	 * \todo These values should be global variables
	 */
	$processUser = posix_getpwuid(posix_geteuid());
	$processUsername = $processUser['name'];
	
	/*!
	 * \todo Solve a major security hole !
	 * \bug This is a major security hole. People can set any value in 'algorithms_app_server' and just pipe comments to ssh !!
	 * 
	 * This is just a minor problem currently, since this code is not distributed and only people having admin access rights can modify
	 * the algorithms_app_server value.
	 * 
	 */
	$command = "ssh-agent  ssh-add " . variable_get('algorithms_ssh_id_file', '~/.ssh/id_rsa') . " && " 
	. "ssh -l '" . variable_get('algorithms_ssh_username', $processUsername) . "' " . variable_get('algorithms_app_server', '') . " 'nohup scripts/DAE_helper.sh "
	. $userID
	. " " 
	. $userEmail
	. " "
	. $runID
	. " "
	. $fullPathToProgram
	. " > log/nohup." . $input['run_id'] . "." . $user->mail . " 2>&1 &' "
	;

	databaseDebugMessage($command); //until we get this working.
	
	system($command, $returnValue);
	return($returnValue);
}
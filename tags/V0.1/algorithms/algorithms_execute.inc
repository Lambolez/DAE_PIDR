<?php 

/*
	algorithms_execute.inc - Last Updated 5/8/10
	
	For the purposes of keeping hardcoded algorithm execution separate, all hardcoded run functions are in this file.
	
	WRITING YOUR METHOD:
	
	To add a procedure for a given algorithm, simply declare a function with the name "run_<algorithm id>($input)" in a similiar fashion to the ones below such that <algorithm id> is the id of the algorithm in the oracle database.
	
	$input is an array of the format $input["file_<input_number>"] = '<filepath>' where the file path is relative to the users uploads directory.  Begin your function by declaring global $uploads; and then you can refer to files by concatenating $uploads and a filepath. $spool and $uploads have no trailing slashes, and filepaths begin with /, so there is no need to concatenate a slash when creating a full path (ex: $uploads . $filepath will produce a correct path)
	
	$input also contains the run id of the algorithm run: $input['run_id']
	
	As of 5/8/10 querying the database for an algorithm's required filetypes is not implemented, so for now there will be only one input ($input["file_0"])  
	
	Note: if you use a variable in place of the 0 because you have more than one file input (input["file_$i"]) you must put the array key in double quotes or the string $i will be passed instead of the value of $i
	
	Printing or echoing any text will cancel the construction of the web page, so simply take care of whatever system calls are necessary.  No return value is necessary.
	
	User id can be accessed by adding global $user; and referring to $user->uid
	
	Look at run_XX for some example code.
	
	REQUIREMENTS:
	
	In order for the webserver to retreive the results correctly, please follow these guidelines:
	
	-Run the method algorithms_create_spool_directories($input) and pass the $input.  This call accomplishes the following:
	
		-Creates a subdirectory of $spool (/dae/spool/<user_id>/<algorithm_run_id>/) named after the algorithm run id ($input['run_id']) resulting a directory structure like /dae/spool/12/450 where 12 is a user id and 450 is the algorithm run id.  Note: $spool = /dae/spool/<user_id>
	
		-Creates an output directory (/dae/spool/<user_id>/<algorithm_run_id>/output)
		
		-Returns path to /dae/spool/<user_id>/<algorithm_run_id> where input belongs and output directory is
	
	-Copy all input files to $spool . "/" . $input['run_id'] (/dae/spool/<user_id>/<algorithm_run_id>/)
	
	-Declare the output directory as $spool . "/" . $input['run_id']. "/output"
	
	-Make sure your system calls/helper script will result in an end time being placed in the database when the run completes
	
	-For debugging purposes, the website does not forward to the runs page so that you can echo variables for testing purposes.  Finished implementations should not have any echos or prints.
*/


function algorithm_execute($id,$input)
{
	global $spool;
	
	$call = "run_" . $id;
	
	if(!is_dir($spool))
		mkdir($spool);
	
	$db = getDAEDatabase();
	$path = $db->r("select PATH from ALGORITHM where ID = " .$id);
	
	$call($path,$input);
}

function algorithms_create_spool_directories($input)
{
	global $spool;
	
	$runDir = $spool . '/' . $input['run_id'];

	//mkdir($runDir);
	
	$outputDir = $runDir. '/output';
	
	mkdir($outputDir,0700,true);
	
	return $runDir;
}
function algorithms_get_file_name($path)
{
	$file = explode("/",$path); //if the path is /mydir/image.png we must remove /mydir
	
	end($file);
	
	return '/'. $file[key($file)];

}

//----------------------------------------------------------------------------------------
/* 
 * This is a hack for compatibility with the standalone MySQL database.
 * Since algorithm IDs are different, the run_XX hack doesn't work.
 * 
 *  Redefining new run_XX functions to conform to MySQL numbering.
 */
function run_2($path,$input) 
{
	run_82($path,$input);
}

function run_XX($path,$input) //just example code/usage for a made up algorithm with id 2
{
	global $uploads, $user, $spool;
	
	$userID = $user->uid;
	$userEmail = $user->mail;
	$runID = $input['run_id']; 

	$fullPathToProgram = "ls -l";
	$fullPathToFile = $uploads.$input["file_0"];
	
	//Performs an ls -l (note that it is in escaped quotes)
	//variable_get grabs the admin setting which right now is dae-sbx.cse.lehigh.edu
	//might just save this for after we un-hardcode these algorithms.
	
	$command = "ssh-agent  ssh-add /opt/csw/home/webservd/.ssh/id_rsa && ssh " . variable_get('algorithms_app_server', '') . " 'nohup " . $path . " "
	. $userID
	. " " 
	. $userEmail
	. " "
	. $runID
	. " "
	. $fullPathToProgram
	. " > log/nohup." . $input['run_id'] . "." . $user->mail . " 2>&1 &' "
	;

	//echo $command; //until we get this working.
	
	system($command);
}

//-----------------------------------------------------------------------------------------

function run_82 ($path,$input) // DICE
{
	global $uploads, $user, $spool;
	
	$runFolder = algorithms_create_spool_directories($input); //creates the proper directories in /dae/spool/<user_id>
	
	$inputFilePath = $uploads.$input['file_0']; //creates path to file in /dae/userdata/<user_id>/uploads/<path_to_file>
	
	$file = algorithms_get_file_name($input['file_0']); //strips the names of any directories from the file path relative to /dae/userdata/<user_id>/uploads/
	
	$spoolInput = $runFolder . $file; //name of the destination for our copy without leaving something like /mydir/image.png
	
	$outputDir = $runFolder . "/output"; //standard output directory is always run location /output
	
	copy($inputFilePath,$spoolInput); //copy file from user directory to spool for running

	//ssh-agent  ssh-add ~/.ssh/id_rsa && ssh dae-sbx /bin/sh dice_scripts/dice_helper.sh <testing image> <results directory> <algorithm run id> <user's email>
	
	$command = "ssh-agent  ssh-add " . variable_get('algorithms_ssh_id_file', '~/.ssh/id_rsa') . " && ssh " . variable_get('algorithms_app_server', '') . " 'nohup " . $path . " "
	. $spoolInput
	. " " 
	. $outputDir
	. " "
	. $input['run_id']
	. " "
	. $user->mail
	. " > log/nohup." . $input['run_id'] . "." . $user->mail . " 2>&1 &' "
	;
	
	echo $command; //until we get this working.
	
	system($command);
}

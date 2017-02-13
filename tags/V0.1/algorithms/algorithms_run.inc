<?php

require_once("./".drupal_get_path('module','daedatabase')."/daedatabase_db.inc");

global $user, $uploads;
$uploads = variable_get('home_directory_userdata', '') . $user->uid . "/uploads";

function algorithms_run()
{
	$db = getDAEDatabase();
	
	if(!$db->checkConcurrentRuns())
	{
		drupal_set_message(t('You may only run up to !num algorithms concurrently.',array('!num' => variable_get('algorithms_concurrent_runs', ''))),'warning');
		return "";
	}
	/*
	
	$requirements = $db->q(" ");
	
	$i = 0;
	
	while($row = oci_fetch_array($requirements,OCI_BOTH))

		$filetypes[$i] = $row['']; //NAME OF FIELD WITH FILE EXTENSIONS
		$i++;

	endwhile;
	
	return $filetypes;
	*/
	
	$filetypes = array('png'); //JUST FOR DICE SO WE CAN HAVE A WORKING DEMO
	
	return algorithms_run_choose(arg(1),$filetypes,$db);
	
}

function algorithms_run_title($al_id)
{	
	$db = getDAEDatabase();
	
	$name = $db->getAlgName($al_id);
	
	drupal_set_title(t('Run !algo',array('!algo' => $name)));
	
	return $name;
}
function algorithms_run_choose($alg,$filetypes,$db)
{
	global $uploads;
	
	//$db = getDAEDatabase();
	
	if(!$db->checkAlgExists($alg))
		return "Algorithm does not exist.";
		
	$name = $db->getAlgName($alg);
	
	drupal_set_title(t('Run !algo',array('!algo' => $name)));
	
	$out = "<form action = \"";
	
	$out .= "?q=run/go"; //! \todo FIX THIS LINK 
	
	$out .= "\" method=\"post\"><ul>";
	
	$i = 0;
	
	foreach($filetypes as $type):
	
		$out .= "Choose a .$type<br/>";
	
		$directories = file_scan_directory($uploads, "\." . $type . "$");
	
		foreach($directories as $dir):
		
			$file =	get_object_vars($dir);
			
			$path = str_replace($uploads, "", $file['filename']);
			
			$out .= "<li><input name=\"file_$i\" value=\"". $path ."\" type=\"radio\">" . $file['basename'] . "</li>";
		
		endforeach;
	
		$i++;
	
	endforeach;

	$out .= "</ul><input name=\"alg_id\" type=\"hidden\" value=\"$alg\"><input value=\"Run\" type=\"submit\"></form>";

	return $out;

}

function algorithms_run_go()
{
	
	global $user;
	
	$webDescription = "";
	
	if(empty($_POST))
	{
		drupal_goto('runs');
		return;
	}
	
		
	$input = $_POST;
	
	$alg_id = $input['alg_id'];
	
	$db = getDAEDatabase();
	
	$insertDate = date("d-M-Y h:i:s A");
	
	/*
	 * ATTENTION ! Are the following lines thead-safe ?
	 * 
	 * Wouldn't it be safer to retrieve SEQ_ALGORITHM_RUN.NEXTVAL in a local variable and use that ?
	 * It seems that Oracle treats this correctly by making these values session-related.
	 * 
	 */
	$inserted_id = $db->createUniqueEntry('ALGORITHM_RUN','ID');
	
	$db->i("update ALGORITHM_RUN set DESCRIPTION = '$webDescription', START_TIME = '$insertDate' where ID = $inserted_id ;");
	
	$db->i("INSERT INTO ALGORITHM_RUN_OF VALUES($inserted_id, $alg_id)");
	
	$db->i("INSERT INTO EXECUTES_RUN VALUES(".$user->uid.", $inserted_id, '$webDescription')");
	
	$input['run_id'] = $inserted_id;
	
	//*/
	/*
	
		Solutions for running each algorithm are hard coded into algorithms_execute.inc
		
		when things are fixed the system call below should handle all algorithms
	
	*/
	//system("ssh " . variable_get('algorithms_app_server', '') . " \"ls -l\"");
	
	require_once('algorithms_execute.inc');
	
	algorithm_execute($alg_id,$input);

	drupal_goto('runs');
	
}

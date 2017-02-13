<?php

function algorithms_copy_run()
{
	$run = arg(2);
	
	if(!algorithms_valid_run($run))
		return "";
	if(!algorithms_run_done($run))
		return "";
	
	$copied = algorithms_copy($run);
	
	drupal_goto("uploads/view/$copied");

}

function algorithms_run_done($run)
{
	global $user;
	
	require_once("./".drupal_get_path('module','algorithms')."/algorithms_status.var");
	
	$db = getDAEDatabase();
	
	$status = $db->r("select STATUS from ALGORITHM_RUN join EXECUTES_RUN on ID = ALGORITHM_RUN_ID where PERSON_ID = ". $user->uid ." and ALGORITHM_RUN.ID = ". $run);
	
	if($status == RUN_COMPLETE):
		return true;
	else:
		drupal_set_message("Run is not complete.","warning");
		return false;
	endif;
}

function algorithms_valid_run($run)
{
	if(is_numeric($run)):
		return true;
	else:
		watchdog("Security",t("A user attempted to use \"!data\" as a run ID",array("!data" => $run)),array(), WATCHDOG_WARNING);
		drupal_set_message("Invalid run ID","warning");
		return false;
	endif;
}

function algorithms_copy($run)
{
	global $runs, $uploads;
	
	$output = "$runs/$run/output";
	
	$files = algorithms_get_output_files($output);
	
	$dir = algorithms_copy_make_dest_dir($run);
	
	foreach($files as $file):
	
		copy("$runs/$run/output/$file","$uploads/$dir/$file");
	
	endforeach;

	return $dir;
}
function algorithms_get_output_files($output)
{
	
	if(!is_dir($output)):
		drupal_set_message("The output was not found.","warning");
		return "";
	endif;
	
	$files = scandir($output);
	
	foreach($files as $key => $file):
	
		if($file == "." || $file == ".." || $file == "output.zip")
			unset($files[$key]);
	
	endforeach;
	
	return $files;
}
function algorithms_copy_make_dest_dir($run)
{
	global $runs, $uploads;
	
	require_once("./".drupal_get_path('module','home_directory')."/home_directory.module");
	
	$db = getDAEDatabase();
	
	$alg_id = $db->r("select algorithm_id from algorithm_run_of join algorithm_run on algorithm_run_id = id where id = ". $run);
	
	$directory = $db->getAlgName($alg_id) . "_" . $run;
	
	return home_directory_make_directory($directory,$uploads.'/',true);
}
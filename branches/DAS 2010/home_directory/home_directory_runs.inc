<?php

require_once("./".drupal_get_path('module','daedatabase')."/daedatabase_db.inc");

global $user, $runs;
$runs = variable_get('home_directory_userdata', '') . $user->uid . "/runs";

function home_directory_runs()
{
	global $runs, $user;
	
	require_once("./".drupal_get_path('module','dae_ui')."/dae_ui.module");
	require_once("./".drupal_get_path('module','algorithms')."/algorithms_status.var");
	
	$db = getDAEDatabase();

	$results = $db->q("select NAME, ALGORITHM_RUN_ID, ALGORITHM_ID, START_TIME, END_TIME, ALGORITHM_RUN.STATUS as STAT, " . 
					$db->dateFormat('END_TIME','MON DD, YYYY') . " as ENDDAY, " . 
					$db->dateFormat('END_TIME','HH:MI:SS') . " as ENDTIME 
					from ALGORITHM_RUN join EXECUTES_RUN on ID = ALGORITHM_RUN_ID natural join ALGORITHM_RUN_OF join ALGORITHM on ALGORITHM.ID = ALGORITHM_ID 
					where PERSON_ID = ". $user->uid ." and ALGORITHM_RUN.STATUS != ".RUN_COMPLETE_DELETED." and ALGORITHM_RUN.STATUS != ".RUN_FAILED_DELETED." order by START_TIME DESC, END_TIME");
	
	$ui = new RunUI("dae-run");

	if(empty($results))
		$a = "a";
	else
	{
		while($row = $db->fetch_array($results)):
	
			$info['left'] = "<a href=\"javascript:show()\">".$row['NAME']." (". get_input_filename($row['ALGORITHM_RUN_ID']) .")</a>";
			$info['children'] = "collap";
			$info['id'] = $row['ALGORITHM_RUN_ID'];
			$info['image'] = "files.png";
			$info['onRowClick'] = "showRunOutput(this,".$row['ALGORITHM_RUN_ID'].")";
			$info['right'] = $ui->getRunStatus($row);
			
			
			$ui->addRecord($info);
	
		endwhile;
	}
	
	return $ui->getOutput();
		
}
function get_input_filename($run) //Almost definitely not stable.
{
	global $spool, $runs;

	if(is_dir($spool . '/' . $run))
		$files = hd_scandir($spool . '/' . $run);
	else
		$files = hd_scandir($runs .'/'. $run . '/');
	
	foreach($files as $file):
		if($file != "output")
			$input = $file;
	endforeach;
	
	return $input;
	
}
function home_directory_get_run_info()
{
	global $user, $spool;
	
	require_once("./".drupal_get_path('module','algorithms')."/algorithms_status.var");
	
	$run = arg(1);
	
	/*  Currently not needed
	if($run == "done")
	{
		$run = arg(2);
		if(is_numeric($run))
		{
			if(home_directory_run_check_done($run))
				echo "true";
			else
				echo "false";
			return;
		}
	}
	*/
	if(!is_numeric($run))
	{
		drupal_set_message(t('Invalid Run ID.'),'warning');
		return "";
	}

	switch(home_directory_run_check_done($run))
	{
	case RUN_RUNNING:
		home_directory_get_run_info_working($run);
		break;
	case RUN_COMPLETE:
		home_directory_copy_to_runs($run);
		home_directory_get_run_info_done($run);
		break;
	case RUN_FAILED:
		//home_directory_copy_to_runs($run);
		//home_directory_get_run_info_done($run);
		home_directory_get_run_info_failed($run);
		break;
	default:
		home_directory_get_run_info_working($run);
		break;
	}
}
function home_directory_get_run_info_failed($run)
{
	echo "Run failed";
}
function home_directory_run_check_done($run)
{
	global $user;
	
	$db = getDAEDatabase();

	$checkDone = $db->r("select STATUS from ALGORITHM_RUN join EXECUTES_RUN on ID = ALGORITHM_RUN_ID where PERSON_ID = ". $user->uid ." and ALGORITHM_RUN.ID = ". $run);

	return $checkDone;
	
}
function home_directory_get_run_info_working($run)
{	
	global $user, $spool, $runs;
	
	$db = getDAEDatabase();
	
	
	
	$elapsed = $db->runElapsedTime($run);
	
	echo "<div class=\"algorithm_run_stats\"><h4>Run Statistics:</h4>	<div class=\"algorithm_run_stats_labels\">
	Start Time:<br/>
	Current Time:<br/>
	Time Elapsed:<br/>
	</div>
	<div class=\"algorithm_run_stats_info\">";
	
	echo $elapsed['start_day']
	." at "
	.$elapsed['start_time']
	."<br/>"
	.$elapsed['cur_day']
	." at "
	.$elapsed['cur_time']
	."<br/>"
	.(int)$elapsed['days']." days, ".$elapsed['time'][0].":".$elapsed['time'][1].":".sprintf("%02.0f",$elapsed['time'][2])."</div>
	</div>";

}
function home_directory_copy_to_runs($run, $folder = "")
{
	global $spool, $user, $runs;
	
	$spoolDir = $spool .'/'. $run;
	
	if(file_exists($spoolDir))
	{
		$files = hd_scandir($spoolDir. '/' .$folder);
		
		if(empty($folder) && !is_dir($runs . '/' . $run))
			mkdir($runs . '/' . $run);
		else if(!is_dir($runs . '/' . $run . '/' . $folder))
			mkdir($runs . '/' . $run . '/' . $folder);
		
		foreach($files as $file):
			
			$from = $spool .'/'. $run .'/'. $folder . $file;
			$to = $runs .'/'. $run . '/' . $folder . $file;
		
			if(is_file($from))
			{
				home_directory_change_usage(filesize($from));
				rename($from,$to);
			}
			else if(is_dir($from))
				home_directory_copy_to_runs($run,$folder.$file.'/');
		
		endforeach;
	}

}
function home_directory_delete_runs_folders($run,$folder = "")
{
	global $spool;
	
	if(file_exists($spool .'/'. $run))
	{
	
		$spoolDir = $spool .'/'. $run .'/' . $folder;
		
		$files = hd_scandir($spoolDir);
		
		foreach($files as $file):
		
			if(is_dir($spoolDir . '/' . $file))
			{
				home_directory_delete_runs_folders($run,$folder.$file."/");
			}
			
		endforeach;
		
		rmdir($spoolDir);
		
	}
}

/**
 * \brief Create a zip file containing all result files of a given run.
 * 
 * This function requires the global variable $runs to be set to the appropriate directory and takes the
 * assumption that $runs/$un/output is a valid and writeable path.
 * 
 * If the zip file is created, the function updates the user's file storage space usage.
 * 
 * @param $run the run id.
 */
function home_directory_create_run_zip($run)
{
	global $runs;
	shell_exec("cd $runs/$run/output && zip -r output.zip *");
	if(file_exists("$runs/$run/output/output.zip"))
		home_directory_change_usage(filesize("$runs/$run/output/output.zip"));
}
function home_directory_get_run_info_done($run)
{
	global $user, $spool, $runs;

	$db = getDAEDatabase();

	if(file_exists($spool .'/'. $run))
	{
		home_directory_copy_to_runs($run);
		home_directory_delete_runs_folders($run);
		home_directory_create_run_zip($run);
	}
	
	echo "<div class=\"algorithm_run_input_files\"><h4>Input Files:</h4> <ul>";
	
	$files = hd_scandir($runs . "/$run");
	
	foreach($files as $file):
	
		if($file != "output")
			echo "<li>"
			.l($file,"getFile/runs/$run/$file")
			."</li>";		
			
	endforeach;
	
	echo "</ul> </div>";
	
	echo "<div class=\"algorithm_run_output_files\"><h4>Output Files: </h4><ul>";//.l("Download All","getFile/runs/$run/output/output.zip")." <ul>";
	
	$files = hd_scandir($runs . "/$run/output");

	foreach($files as $file):
	
		if($file != "output.zip")
			echo "<li>"
			.l($file,"getFile/runs/$run/output/$file")
			."</li>";

	endforeach;
	

	echo "</ul> </div>";

	$elapsed = $db->runLength($run);
	
	echo "<div class=\"algorithm_run_stats\">
			<h4>Run Statistics:</h4>
			<div class=\"algorithm_run_stats_labels\">
				Start Time:<br/>
				End Time:<br/>
				Run Length:<br/>
			</div>
			<div class=\"algorithm_run_stats_info\">"
				.$elapsed['start_day']
				." at "
				.$elapsed['start_time']
				."<br/>"
				.$elapsed['end_day']
				." at "
				.$elapsed['end_time']
				."<br/>"
				.(int)$elapsed['days']
				." days, "
				.$elapsed['time'][0]
				.":".$elapsed['time'][1]
				.":"
				.sprintf("%02.0f",$elapsed['time'][2])
			."</div>
			</div></div>";
	
}
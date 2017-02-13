<?php

require_once("./".drupal_get_path('module','daedatabase')."/daedatabase_db.inc");

global $user, $runs;
$runs = variable_get('home_directory_userdata', '') . $user->uid . "/runs";

function home_directory_runs()
{
	global $runs, $user;

	$db = getDAEDatabase();

	$results = $db->q("select NAME, ALGORITHM_RUN_ID, ALGORITHM_ID, START_TIME, END_TIME, " . 
					$db->dateFormat('END_TIME','MON DD, YYYY') . " as ENDDAY, " . 
					$db->dateFormat('END_TIME','HH:MI:SS') . " as ENDTIME 
					from ALGORITHM_RUN join EXECUTES_RUN on ID = ALGORITHM_RUN_ID natural join ALGORITHM_RUN_OF join ALGORITHM on ALGORITHM.ID = ALGORITHM_ID 
					where PERSON_ID = ". $user->uid ." order by START_TIME DESC, END_TIME");
	
	$i = 1;

	if(empty($results))
		$output .= "<div class=\"hd-runs_row\"><span class=\"left\">You have no runs for query $thequery.</span></div>";
	else
		while($row = $db->fetch_array($results)):

	$alg_id = $row['ALGORITHM_ID'];
		
	$output .= "<div class=\"hd-runs_row" . ($i%2==1?" gray":"") . "\">"; //container
		
	$output .= "<div class=\"hd-runs_block\" onClick=\"showRunOutput(". $row['ALGORITHM_RUN_ID'] .")\"><span class=\"left\">"; //always visible title

	$output .= "<h5>" . $row['NAME'] . "</h5>";

	$output .= "</span>";

	if(empty($row['END_TIME']))
		$output .= "<span id=\"run_status-".$row['ALGORITHM_RUN_ID']."\" class=\"algorithm_run_running\">In Progress&nbsp;&nbsp;<img id=\"run_status_image-".$row['ALGORITHM_RUN_ID']."\" src=\"".drupal_get_path("module","home_directory")."/images/running.gif\" alt=\"Running\" />&nbsp;</span>";
	else
		$output .= "<span class=\"algorithm_run_complete\">Complete <img src=\"".drupal_get_path("module","home_directory")."/images/check.png\" alt=\"Done\" /></span>";

	$output .= "</div>"; //end title block

	$output .= "<div id=\"output-". $row['ALGORITHM_RUN_ID'] ."\" class=\"algorithm_run_output\"><img src=\"".drupal_get_path("module","home_directory")."/images/ajax.gif\" alt=\"\" /> </div></div>";

	$i++;
		
	endwhile;

	return $output;

}
function home_directory_get_run_info()
{
	global $user, $spool;

	$run = arg(1);

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

	if(!is_numeric($run))
	{
		drupal_set_message(t('Invalid Run ID.'),'warning');
		return "";
	}

	if(!home_directory_run_check_done($run))
	home_directory_get_run_info_working($run);
	else
	{
		home_directory_copy_to_runs($run);
		home_directory_get_run_info_done($run);
	}
}
function home_directory_run_check_done($run)
{
	global $user;

	$db = getDAEDatabase();

	$checkDone = $db->r("select count(*) from ALGORITHM_RUN join EXECUTES_RUN on ID = ALGORITHM_RUN_ID where PERSON_ID = ". $user->uid ." and ALGORITHM_RUN.ID = ". $run . " and END_TIME is not null");

	return ($checkDone > 0);

}
function home_directory_get_run_info_working($run)
{
	global $user, $spool, $runs;

	$db = getDAEDatabase();

	$elapsed = $db->runElapsedTime($run);

	echo "<div class=\"algorithm_run_stats\"><h4>Run Statistics:</h4><br/>	<div class=\"algorithm_run_stats_labels\">
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
function home_directory_create_run_zip($run)
{
	global $runs;
	shell_exec("cd $runs/$run/output
	zip -r output.zip *");
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

	echo "<div class=\"algorithm_run_output_files\"><h4>Output Files: </h4>".l("Download All","getFile/runs/$run/output/output.zip")." <ul>";

	$files = hd_scandir($runs . "/$run/output");

	foreach($files as $file):

	if($file != "output.zip")
	echo "<li>"
	.l($file,"getFile/runs/$run/output/$file")
	."</li>";

	endforeach;

	echo "</ul> </div>";

	$elapsed = $db->runLength($run);

	echo "<div class=\"algorithm_run_stats\"><h4>Run Statistics:</h4><br/>
	<div class=\"algorithm_run_stats_labels\">
	Start Time:<br/>
	End Time:<br/>
	Run Length:<br/>
	</div>
	<div class=\"algorithm_run_stats_info\">";


	echo $elapsed['start_day']
	." at "
	.$elapsed['start_time']
	."<br/>"
	.$elapsed['end_day']
	." at "
	.$elapsed['end_time']
	."<br/>"
	.(int)$elapsed['days']." days, ".$elapsed['time'][0].":".$elapsed['time'][1].":".sprintf("%02.0f",$elapsed['time'][2])."</div>
	</div>
	</div>";	

}

function hd_scandir($dir)
{
	/**
	 * Normally the $dir should always exist -execept for testing environments- 
	 * 
	 * \todo Log an error in the appropriate log file
	 */
	if(file_exists($dir)) {
		$files = scandir($dir);
		unset($files[0]);
		unset($files[1]);
		return $files;
	}
	else return array();
}
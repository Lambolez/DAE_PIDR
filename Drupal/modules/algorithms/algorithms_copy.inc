<?php
/**
 * @file
 * 
 * @author  Mike Kot <mdk312@lehigh.edu>
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
 *
 */
 
 /**
 * \brief Copies the output of a run from a user's uploads directory to their uploads directory
 * 
 * Confirms that the run exists and is finished.  Creates a directory with the name of the run
 * underscore run id in My Uploads
 * 
 */
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
/**
 * \brief Confirms a run is finished
 * 
 * Requires run status vars and the database class
 * 
 * @param $run the run id.
 */
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
/**
 * \brief Actually copies the files between runs and uploads
 * 
 * Creates a directory to place the output files named algorithm_run-id
 * 
 * @param $run the run id.
 */
function algorithms_copy($run)
{
	global $runs, $uploads;
	
	$output = "$runs/$run/output";
	
	require_once("./".drupal_get_path('module','algorithms')."/algorithms_my_runs.inc");
	
	algorithms_check_files_moved_to_runs($run);
	
	$files = algorithms_get_output_files($output);
	
	$dir = algorithms_copy_make_dest_dir($run);
	
	foreach($files as $file):
	
		copy("$runs/$run/output/$file","$uploads/$dir/$file");
		home_directory_change_usage(filesize("$uploads/$dir/$file"));
	
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
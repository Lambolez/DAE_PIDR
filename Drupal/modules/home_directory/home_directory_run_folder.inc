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
 * \brief Copies files from a spool directory to the user's run directory
 * 
 * This function requires the global variables $runs and $spool to be set to the appropriate directories 
 * 
 * Increases user's used storage space as it copies files and directory structure
 * 
 * @param $run the run id.
 * @param $folder used for recursive deletes. Not necessary on initial call
 */
function home_directory_copy_to_runs($run, $folder = "")
{
	global $spool, $user, $runs;
	
	require_once("./".drupal_get_path('module','home_directory')."/home_directory.module");
	
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
/**
 * \brief Recursively deletes all the directories in the spool run folder
 * 
 * This function requires the global variable $spool to be set to the appropriate directory
 * 
 * @param $run the run id.
 * @param $folder used for recursive deletes. Not necessary on initial call
 */
function home_directory_delete_runs_folders($run,$folder = "")
{
	global $spool;
	
	require_once("./".drupal_get_path('module','home_directory')."/home_directory.module");
	
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
 * assumption that $runs/$run/output is a valid and writeable path.
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
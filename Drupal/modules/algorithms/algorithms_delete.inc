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
function algorithms_delete_run()
{
	$run = arg(2);
	
	if(!algorithms_valid_run($run)) //Check for SQL injection
		return "";
	
	if(!algorithms_delete_access($run)) //Check user owns run
		return "";
		
	db_query("UPDATE {runs_deleted} SET deleted = 1 WHERE run_id = " . $run);

	algorithms_delete_run_files($run);
	
	echo "Run Deleted Successfully";
	
	//drupal_goto('runs'); //Called via ajax, therefore redirect not necessary
}

function algorithms_delete_access($run)
{
	global $user;
	
	$db = getDAEDatabase();
	
	$owner = $db->r("select PERSON_ID from EXECUTES_RUN where ALGORITHM_RUN_ID = ". $run);
	
	if($owner == $user->uid):
		return true;
	else:
		drupal_set_message("You do not have access to this run.","warning");
		return false;
	endif;
}
function algorithms_delete_run_files($run)
{
	global $runs,$spool;	
	
	require_once("./".drupal_get_path('module','algorithms')."/algorithms_status.var");
	
	$db = getDAEDatabase();
	
	$status = $db->r("select STATUS from ALGORITHM_RUN where ID = ". $run);
	
	if($status == RUN_COMPLETE)
	{
		if(is_dir("$runs/$run"))
		{
			$size = intval(shell_exec("du -sk $runs/$run"));
			home_directory_change_usage(-($size*1024));
			shell_exec("rm -r $runs/$run");
		}
		else if(is_dir("$spool/$run"))
		{
			shell_exec("rm -r $spool/$run");
		}
	}
	else if($status < 0)
	{
		if(is_dir("$spool/$run"))
		{
			shell_exec("rm -r $spool/$run");
		}
	}
	
}

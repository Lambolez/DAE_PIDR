<?php

require_once("./".drupal_get_path('module','daedatabase')."/daedatabase_db.inc");
require_once("./".drupal_get_path('module','algorithms')."/algorithms_status.var");


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
	$al = arg(1);
	
	if(!is_numeric($al))
	{
		drupal_set_message(t('Invalid algorithm id.'),'warning');
		return "";
	}

	/**
	 * Get list of file extensions allowed for this algorithm
	 */
	$requirements = $db->q("select distinct P.VALUE from ASSOCIATE_ALGO_RESTRICTION A, PAGE_IMAGE_RESTRICTION P, ALGORITHM ALGO
where ALGO.ID = A.ALGORITHM_ID and A.RESTRICTION_ID = P.ID and ALGO.ID = " . $al);

	$i = 0;
	
	while($row = $db->fetch_array($requirements,OCI_BOTH)):

		$filetypes[$i] = $row['VALUE'];
		$i++;

	endwhile;
		
	return algorithms_run_choose($al,$filetypes);
		
}

function algorithms_run_title($al_id)
{	
	$db = getDAEDatabase();
	
	$name = $db->getAlgName($al_id);
	
	drupal_set_title(t('Run !algo',array('!algo' => $name)));
	
	return $name;
}
function algorithms_run_choose($alg,$filetypes)
{
	
	$db = getDAEDatabase();
	
	if(!$db->checkAlgExists($alg))
		return "Algorithm does not exist.";
		
	$name = $db->getAlgName($alg);
	
	drupal_set_title(t('Run !algo',array('!algo' => $name)));
	
	$out = "<form action = \"";
	
	$out .= "?q=run/go"; //! \todo FIX THIS LINK 
	
	$out .= "\" method=\"post\"><ul>";
	
	$file_no = 0;
	
	foreach($filetypes as $type):
	
		switch($type)
		{
		case "type the output format":
			$out .= "Choose an output format<br/>";
			$out .= algorithms_get_convert_output_formats($file_no);
			break;
		case "*.*":
			$out .= "Choose any file<br/>";
			$out .= algorithms_scan_directory_for_extension("*",$file_no);
			break;
		default:
			$out .= "Choose a .$type<br/>";
			$out .= algorithms_scan_directory_for_extension($type,$file_no);
			break;
		}
		$file_no++;
	
	endforeach;

	$out .= "</ul><input name=\"alg_id\" type=\"hidden\" value=\"$alg\"><input value=\"Run\" type=\"submit\"></form>";

	return $out;

}
function algorithms_get_convert_output_formats($input_no)
{
	$types = explode(" ",variable_get('algorithms_convert_output', ''));
	$output = "<select name=\"file_$input_no\">";
	
	foreach($types as $type):
	
		$output .= "<option value=\".$type\">$type</option>";
	
	endforeach;
	
	$output .= "</select><br/><br/>";
	
	return $output;
}

function algorithms_scan_directory_for_extension($type,$file_no)
{
	global $uploads;
	
	$directories = file_scan_directory($uploads, "\." . $type . "$");
	
	foreach($directories as $dir):
	
		$file =	get_object_vars($dir);
		
		$path = str_replace($uploads, "", $file['filename']);
		
		$out .= "<li><input name=\"file_$file_no\" value=\"". $path ."\" type=\"radio\">" . $file['basename'] . "</li>";
	
	endforeach;
	
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
	
	//print_r($_POST);
	
	$alg_id = $input['alg_id'];
	
	$db = getDAEDatabase();
	
	$insertDate = date($db->timestampFormat); 

	$inserted_id = $db->createUniqueEntry('ALGORITHM_RUN','ID');
	
	$db->i("update ALGORITHM_RUN set DESCRIPTION ='$webDescription', START_TIME ='$insertDate', STATUS = ".RUN_RUNNING." where ID = $inserted_id");
	
	$db->i("INSERT INTO ALGORITHM_RUN_OF VALUES($inserted_id, $alg_id)");
	
	$db->i("INSERT INTO EXECUTES_RUN VALUES(".$user->uid.", $inserted_id, '$webDescription')");
	
	$input['run_id'] = $inserted_id;
	
	/**
	 * @todo Solutions for running each algorithm are hard coded into algorithms_execute.inc
	 * when things are fixed the system call below should handle all algorithms
	*/	
	require_once('algorithms_execute.inc');
	
	/**
	 * \todo check if we can store the return value as STATUS in the database
	 */
	$systemReturnValue = algorithm_execute($alg_id,$input);
	if($systemReturnValue != 0)
	{
		$insertDate = date($db->timestampFormat); 
		//databaseDebugMessage("update ALGORITHM_RUN set END_TIME = '$insertDate', STATUS = -1 where ID = $inserted_id");
		$db->i("update ALGORITHM_RUN set END_TIME = '$insertDate', STATUS ='-$systemReturnValue' where ID = $inserted_id");
	}
	
	drupal_goto('runs');
	
}

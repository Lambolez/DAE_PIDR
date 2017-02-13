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
 * 
 */
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
	$al[':algo'] = arg(1);
	
	if(!is_numeric($al))
	{
		drupal_set_message(t('Invalid algorithm id.'),'warning');
		return "";
	}

	/**
	 * Get list of file extensions allowed for this algorithm
	 */
	
	$requirements = $db->q("select distinct P.VALUE from ASSOCIATE_ALGO_RESTRICTION A, PAGE_IMAGE_RESTRICTION P, ALGORITHM ALGO
where ALGO.ID = A.ALGORITHM_ID and A.RESTRICTION_ID = P.ID and ALGO.ID = :algo order by P.VALUE",$al);

	$i = 0;
	
	while($row = $db->fetch_array($requirements,OCI_BOTH)):

		$filetypes[$i] = $row['VALUE'];
		$i++;

	endwhile;
		
	return algorithms_run_choose($al[':algo'],$filetypes);
		
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
	$on = array(arg(3)); $as = array(arg(5));
	
	$db = getDAEDatabase();
	
	if(!$db->checkAlgExists($alg))
		return "Algorithm does not exist.";
		
	$name = $db->getAlgName($alg);
	
	drupal_set_title(t('Run !algo',array('!algo' => $name)));
	
	if(is_numeric($on[0]) && is_numeric($as[0]))
		return algorithm_pre_selected($alg,$on,$as);
		
		
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

/**
 * 
 * @param [in] $alg
 * @param [in] $on
 * @param [in] $as
 */
function algorithm_pre_selected($alg,$on,$as)
{
	require_once(drupal_get_path("module","dae_data")."/dae_data.module");
	drupal_add_js("misc/collapse.js");
	require_once(drupal_get_path("module","dae_data")."/dae_data.module");
	dae_data_increment_activity($on[0],4);
	
	$form = drupal_get_form('algorithms_run_form');

	return $form;
}
function validatePreSelected()
{
	$alg = arg(1);
	
	if(!is_numeric($alg)):
		drupal_set_message("Invalid Algorithm ID",'warning');
		return false;
	endif;
	
	$db = getDAEDatabase();
	
	if(!$db->checkAlgExists($alg)):
		drupal_set_message("Algorithm does not exist.","warning");
		return false;
	endif;
	
	$output['algorithm_id'] = $alg;
	
	$valid = false;
	$i = 2;
	$input = arg($i);
	
	while($input):
	
		$on = arg($i+1);
		$as = arg($i+3);
		if(is_numeric($on) && is_numeric($as)):
			$output['on'][] = $on;
			$output['as'][] = $as;
			$valid = true;
		endif;
		$i+=4;
		$input = arg($i);
			
	endwhile;
	
	if($valid)
		return $output;
	else
		return false;
}
function file_extension_dropdown($row)
{
	$types = explode(" ",variable_get('algorithms_convert_output', ''));
	
	foreach($types as $type):
	
		$filetypes[$type] = $type;
	
	endforeach;
	 
	 $form = array(
    		 '#type' => 'select',
			  '#title' => $row['DESCRIPTION'],
			  '#options' => $filetypes,
			  '#required' => true
			);
	return $form;
}
function algorithms_run_form()
{
	drupal_add_js('misc/collapse.js');
	
	$db = getDAEDatabase();
	
	$preSelected = validatePreSelected();
	extract($preSelected);
	
	$variables[':algo'] = $algorithm_id;
	$input = $db->q("select DESCRIPTION,DATA_ID from ALGORITHM_INPUT A join DATA D on A.DATA_ID = D.ID where ALGORITHM_ID = :algo and REQUIRED = 1 ORDER BY ORDERING",$variables);
	
	$form['algorithm_id'] = array(
							'#type' => 'hidden',
							'#value' => t('!d',array('!d' => $algorithm_id)),
							
							);
	$form['#redirect'] = 'runs';
	
	$form['r-block-start'] = array('#type' => 'markup', 
								'#value' => "<fieldset class=\"collapsible\">
								<legend class=\"collapse-processed\">
									<a href=\"#\"  onClick=\"Drupal.toggleFieldset(this.parentNode.parentNode);event.stopPropagation()\">Required</a>
								</legend>
								<div id=\"algorithm_run_form\" class=\"fieldset-wrapper\" style=\"display: block; \">"
								);
	
	while($row = $db->fetch_array($input)):
	
		if($row['DESCRIPTION'] == "Choose a filetype to convert to"):
			$form['algorithm_run_input-'.$row['DATA_ID']] = file_extension_dropdown($row);
		
		elseif(!in_array($row['DATA_ID'],$as)):
			 $form['algorithm_run_input-'.$row['DATA_ID']] = array(
	    		 '#type' => 'textfield',
				  '#title' => 'input',
				  '#default_value' => $object['foo'],
				  '#size' => 60,
				  '#maxlength' => 64,
				  '#description' => $row['DESCRIPTION'],
  				);
	
  		else:
  			$key = array_search($row['DATA_ID'],$as);
  			
  			// !todo make sure $on[$key] is suitable input.  Here or validate?
  			
  			$form['algorithm_run_input-'.$as[$key]] = array(
	    		'#type' => 'radios', 
	  			'#required' => TRUE,
	   			'#default_value' => 'selected',
	   			'#return_value' => $on[$key],
	   			'#attributes' => array('checked' => 'checked'),
	   			'#title' => $row['DESCRIPTION'],
	   			'#options' => array($on[$key] => dae_data_browse_dataset_dataitem_build($on[$key],true)),
  				);
  			 
		endif;
		
	endwhile;
	
	$form['r-block-end'] = array('#type' => 'markup', '#value' => '</div></fieldset>');
	
	$input = $db->q("select DESCRIPTION,DATA_ID from ALGORITHM_INPUT A join DATA D on A.DATA_ID = D.ID where ALGORITHM_ID = $algorithm_id and (REQUIRED !=  1 or REQUIRED is NULL)");
	
	$form['a-block-start'] = array('#type' => 'markup', 
								'#value' => "<fieldset class=\"collapsible collapsed\">
								<legend class=\"collapse-processed\">
									<a href=\"#\" onClick=\"Drupal.toggleFieldset(this.parentNode.parentNode);event.stopPropagation()\">Advanced Options</a>
								</legend>
								<div class=\"fieldset-wrapper\" style=\"display: block; \">"
								);
	
	while($row = $db->fetch_array($input)):
		
		if(!in_array($row['DATA_ID'],$as)):
			 $form['algorithm_run_input-'.$row['DATA_ID']] = array(
    			 '#type' => 'textfield',
				  '#title' => 'input',
				  '#default_value' => $object['foo'],
				  '#size' => 60,
				  '#maxlength' => 64,
				  '#description' => $row['DESCRIPTION'],
 		 );
	
  		else:
  			$key = array_search($row['DATA_ID'],$as);
  			
  			// !todo make sure $on[$key] is suitable input.  Here or validate?
  			
  			$form['algorithm_run_input-'.$as[$key]] = array(
	    		'#type' => 'radios', 
	  			'#required' => TRUE,
	   			'#default_value' => 'selected',
	   			'#return_value' => $on[$key],
	   			'#attributes' => array('checked' => 'checked'),
	   			'#prefix' => "<label>".$row['DESCRIPTION'].":</label>",
	   			'#options' => array($on[$key] => dae_data_browse_dataset_dataitem_build($on[$key],true)),
  				);
  			 
		endif;
		
	endwhile;
	
	$form['a-block-end'] = array('#type' => 'markup', '#value' => '</div></fieldset>');
	
	$form['algoritm_run_input_submit'] = array(
	    '#type' => 'submit',
	    '#title' => t('Submit'),
	    '#default_value' => t('Run'),
	    '#description' => t('Submit')
  	);
	
	return $form;
}
/**
 * Implementation of algorithms_run_form_validate
 */
function algorithms_run_form_validate($form, &$form_state)
{
}
/**
 * Implementation of algorithms_run_form.
 */
function algorithms_run_form_submit($form_id, $form_values)
{	
	global $user;
	
	$input['id'] = $form_values['values']['algorithm_id'];
	$id = $input['id'];
	
	$db = getDAEDatabase();
	
	$variables[':algo'] = $input['id'];
	$alg_input = $db->q('SELECT DATA_ID from ALGORITHM_INPUT where ALGORITHM_ID = :algo',$variables);
	
	$inserted_id = $db->createUniqueEntry('ALGORITHM_RUN','ID');
	
	$update_variables[':inserted'] = $inserted_id;
	$update_variables[':insertdate'] = date($db->timestampFormat); 
	$update_variables[':description'] = $webDescription;
	$db->i("update ALGORITHM_RUN set DESCRIPTION =:description, START_TIME =:insertdate, STATUS = ".RUN_RUNNING." where ID = :inserted", $update_variables);
	
	$variables[':inserted'] = $inserted_id;
	$db->i('INSERT INTO ALGORITHM_RUN_OF VALUES(:inserted, :algo)', $variables);
	
	$execute_variables[':user_id'] = getPID($user->uid);
	$execute_variables[':inserted'] = $inserted_id;
	$execute_variables[':description'] = $webDescription;
	$db->i("INSERT INTO EXECUTES_RUN VALUES(:user_id, :inserted, :description)",$execute_variables);
	
	$i = 0;
	
	$inputId = '';
	$insert_variables[':inserted'] = $inserted_id;
	$insert_variables[':inputid'] = $inputId;
	
	while($row = $db->fetch_array($alg_input)):
		
	    $inputId = $form_values['values']['algorithm_run_input-'.$row['DATA_ID']];
		$input["file_".$i] = $inputId;
		//drupal_set_message("INSERT INTO ALGORITHM_RUN_INPUT (ALGORITHM_RUN_ID, DATA_ITEM_ID) values ($inserted_id,".$form_values['values']['algorithm_run_input-'.$row['DATA_ID']]." )",'warning');
		if(is_numeric($input["file_".$i])) //hack to stop file extension in convert from gumming up works
			$db->i("INSERT INTO ALGORITHM_RUN_INPUT (ALGORITHM_RUN_ID, DATA_ITEM_ID) values (:inserted, :inputid )", $insert_variables);
		$i++;
	
	endwhile;

	$input['run_id'] = $inserted_id;
	
	db_query("insert into {runs_deleted} values($inserted_id,0)");
	
	/**
	 * @todo Solutions for running each algorithm are hard coded into algorithms_execute.inc
	 * when things are fixed the system call below should handle all algorithms
	*/	
	require_once('algorithms_execute.inc');
	
	/**
	 * \todo check if we can store the return value as STATUS in the database
	 */
	$systemReturnValue = algorithm_execute($input,true);
	
	if($systemReturnValue != 0)
	{
	    unset($update_variables['description']);
	    $update_variables[':sysvalue'] = -$systemReturnValue;
	    $update_variables[':insertdate'] = date($db->timestampFormat); 
		$db->i("update ALGORITHM_RUN set END_TIME =:insertdate, STATUS =:sysvalue where ID = :inserted", $update_variables);
	}
	
	watchdog('debug',$systemReturnValue." - return value");
}
function algorithms_get_convert_output_formats($input_no)
{
	$types = explode(" ",variable_get('algorithms_convert_output', ''));
	$output = "<select name=\"file_$input_no\">";
	
	foreach($types as $type):
	
		$output .= "<option value=\"$type\">$type</option>";
	
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

function algorithms_run_go($input = array())
{
	
	global $user;
	
	$webDescription = "";
	
	if(empty($_POST))
	{
		drupal_goto('runs');
		return;
	}
	
	$input = $_POST;
	
	$alg_id = $_POST['alg_id'];
	$input['id'] = $alg_id;
	
	$db = getDAEDatabase();
	
	$insertDate = date($db->timestampFormat); 
	$inserted_id = $db->createUniqueEntry('ALGORITHM_RUN','ID');
	
	$update_variables[':description'] = $webDescription;
	$update_variables[':insertdate'] = $insertDate;
	$update_variables[':inserted'] = $inserted_id;
	
	$db->i("update ALGORITHM_RUN set DESCRIPTION =:description, START_TIME =:insertdate, STATUS = ".RUN_RUNNING." where ID = :inserted");
	
	$variables[':inserted'] = $inserted_id;
	$variables[':algo'] = $alg_id;
	$db->i('INSERT INTO ALGORITHM_RUN_OF VALUES(:inserted, :algo)', $variables);
	
	$execute_variables[':user_id'] = getPID($user->uid);
	$execute_variables[':inserted'] = $inserted_id;
	$execute_variables[':description'] = $webDescription;
	$db->i("INSERT INTO EXECUTES_RUN VALUES(:user_id, :inserted, :description)",$execute_variables);
	
	$input['run_id'] = $inserted_id;
	
	db_query("insert into {runs_deleted} values($inserted_id,0)");
	
	/**
	 * @todo Solutions for running each algorithm are hard coded into algorithms_execute.inc
	 * when things are fixed the system call below should handle all algorithms
	*/	
	require_once('algorithms_execute.inc');
	
	/**
	 * \todo check if we can store the return value as STATUS in the database
	 */
	$systemReturnValue = algorithm_execute($input,false);
	if($systemReturnValue != 0)
	{
	    unset($update_variables['description']);
	    $update_variables[':sysvalue'] = -$systemReturnValue;
	    $update_variables[':insertdate'] = date($db->timestampFormat); 
		$db->i("update ALGORITHM_RUN set END_TIME =:insertdate, STATUS =:sysvalue where ID = :inserted", $update_variables);
	}
	
	drupal_goto('runs');
	
}

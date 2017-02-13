<?php

/**
 * @file data_repository.inc
 * @author Bart Lamiroy
 * @date May 2010
 */

require_once("./".drupal_get_path('module','daedatabase')."/daedatabase_db.inc");

/**
 * This is just a hack ... 
 */
function data_repository_list() 
{
	$db = getDAEDatabase();
	
	
	if(user_access('view public data'))
		$algoResultSet = $db->q("select distinct NAME,ID from DATASET where ID not in (select ASSOCIATING_DATASET_ID from ASSOCIATE_DATASET)");
	else
		$algoResultSet = $db->q("select distinct NAME,ID from DATASET where ID not in (select ASSOCIATING_DATASET_ID from ASSOCIATE_DATASET)"); //ADD where clause for public checkbox

	
	$output = "<ul id=\"hd-algorithms\">";
	
	/**
	 * \todo use proper CSS and not just the ones of the algorithm module
	 */
	while($row = $db->fetch_array($algoResultSet,DATABASE_BOTH)):
	
		$output .= "<li><strong>" . $row['PATH'] . "</strong> ";
		
		$output .= "<div id=\"data_page_image".$row['ID']."\" class=\"hd-algo_desc\">";
		
		$output .= "<br/>";
		
		$output .= "</div></li>";
	
	endwhile;
	
	$output .= "</ul>";
	
	return $output;
	
}

function data_repository_query()
{
	if(!isset($_SESSION['data_repository']))
    {
    	$_SESSION['data_repository'] = array();
    }
    
	
	$output .= drupal_get_form('data_repository_sql');
	
	return $output;
}
	
function data_repository_sql_submit($form, &$form_state) {
	
	/**
	 * use of _SESSION variables is necessary since globals are only valid within the scope of a page.
	 * In our case we have Repository and Repository/SQL being two different pages. Globals are therefore
	 * not propagated
	 */
	$_SESSION['data_repository']['formQuery'] = $form_state['values']['query'];
	/*
	$out = '<form action = "';	
	$out .= '?q=Repository/SQL';  
	$out .= '" method="post"><ul>';
	$out .= '</ul><input name="query" type="hidden" value="'..'"><input value="Run" type="submit"></form>';
	*/
	
	$form_state['redirect'] = 'Repository/SQL';  
}

function data_repository_display()
//function data_repository_sql_submit($form, &$form_state)
{
	
	if(!user_access('raw DAE database access')) {
		drupal_set_message("Raw Database Access Violation !",error);
		return;
	}
	
	$db = getDAEDatabase();
	$conn = $db->getConn();
	
	//$input = $_POST;
	//$query = $form_state['values']['query'];
	$query = $_SESSION['data_repository']['formQuery']; //$GLOBALS['formQuery'];
	
	$output = '<h3>Output for query "<i>' . $query . '</i>"</h3>';
	
	//$output .= drupal_get_form('data_repository_sql');
	
	switch ($db->dbmsType) {
    case "MySQL":
        databaseDebugMessage("MySQL access not implemented yet");
        break;
    case "Oracle":
    	
		$stmt = oci_parse($conn, $query);

		oci_execute($stmt);
    	$output .= "<p>";

		$output .= '<table border="1">';
		$output .= '<tr>';
    	
    	$ncols = oci_num_fields($stmt);
    	for ($i = 1; $i <= $ncols; $i++) {
        		$column_name  = oci_field_name($stmt, $i);
        		$output .= '<th>' . $column_name .  '</th>';
    		}
    	
    	$output .= "</tr>";
    	
		while (oci_fetch($stmt)) {

			$output .= '<tr>';
			
			for ($i = 1; $i <= $ncols; $i++) {
        		$column_name  = oci_field_name($stmt, $i);
        		$column_value = oci_result($stmt, $i);
        		$output .= '<td>' . $column_value . "</td>";
    		}
    		
    		$output .= "</tr>";
    		
		}

		$output .= "</table>";
		$output .= "</p>";
		
		oci_free_statement($stmt);
	}	
	
	return $output;
}

function data_repository_sql($form_state)
{
  $form = array();
  $form['query'] = array(
    '#type' => 'textarea',
    '#title' => t('SQL Query'),
    '#default_value' => 'select * from PAGE_IMAGE',
    '#description' => t('SQL Query to submit to server')
  );
  $form['submit'] = array('#type' => 'submit', '#value' => t('Submit Query'));
  
  return $form;
}


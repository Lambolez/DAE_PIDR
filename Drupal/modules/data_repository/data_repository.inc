<?php

/**
 * @file data_repository.inc
 * 
 * @author  Bart Lamiroy <lamiroy@cse.lehigh.edu>
 * @date May 2010
 * 
 * @version 1.0
 *
 * \brief Functions handling raw SQL interaction with the backend data base through Drupal
 * 
 * This file contains all necessary code to implement the \link data_repository.module Data Repository Module\endlink 
 * which allows authorized Drupal users to interact directly with the backend data base management system using raw, 
 * unfiltered SQL.
 * 
 * Users need to have the Drupal 'raw DAE database access' access rights set.
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
 

require_once("./".drupal_get_path('module','daedatabase')."/daedatabase_db.inc");

/**
 * \brief Drupal menu callback for Repository as declared in data_repository_menu()
 * 
 */
function data_repository_query()
{
	//! Creates a $_SESSION variable for memorizing cross-page query inputs.
	if(!isset($_SESSION['data_repository']))
    {
    	$_SESSION['data_repository'] = array();
    }
	
    //! Displays the form defined in data_repository_sql().
	$output .= drupal_get_form('data_repository_sql');
	
	//! Drupal automagically attaches data_repository_sql_submit() as the form submit callback.
	return $output;
}

/**
 * \brief Standard Drupal form submit callback function
 * 
 * Redirects the handling of the form submission to 'Repository/SQL', handled by 
 * the data_repository_display() callback.
 * 
 * @param [in] $form Drupal form structure
 * @param [in] $form_state form contents
 */
function data_repository_sql_submit($form, &$form_state) {
	
	/**
	 * use of _SESSION variables is necessary since globals are only valid within the scope of a page.
	 * In our case we have Repository and Repository/SQL being two different pages. Globals are therefore
	 * not propagated
	 */
	$_SESSION['data_repository']['formQuery'] = $form_state['values']['query'];
	
	$form_state['redirect'] = 'Repository/SQL';  
}

/**
 * \brief Drupal menu callback for Repository/SQL as declared in data_repository_menu()
 * 
 * Requires Drupal user to have 'raw DAE database access' access rights.
 * 
 */
function data_repository_display()
{
	
	if(!user_access('raw DAE database access') && !user_access('raw read-only DAE database access')) {
		drupal_set_message("Raw Database Access Violation !",error);
		return data_repository_query();
	}
	
	$db = getDAEDatabase();
	$conn = $db->getConn();
	
    /*
     * Re-displaying the query input form
     */
	$output = drupal_get_form('data_repository_sql');
	
	/*
	 * Retrieving the typed in query
	 */
	$query = $_SESSION['data_repository']['formQuery']; //$GLOBALS['formQuery'];
	
	/*
	 * Dumb, plain HTML header display
	 * \todo Make this display CSS-ed
	 */
	$output .= '<h3>Output for query "<i>' . $query . '</i>"</h3>';
	
    // Parse query
    $stmt = oci_parse($db->getConn(),$query);
	if (!oci_execute($stmt))
		databaseDebugMessage($query,false);
	
    $output .= "<p>";

    // Creating a dumb HTML table to display the results
    /*!
     * \todo Make the HTML disply more appealing and efficient. Current version can be fery slow and
     * memory consuming on queries with very large output. Might be a good idea to have a generic
     * way of handling bulky query outputs all over the project 
     */
    // 1. Creating the HTML Table header	  
	$output .= '<table border="1">';
	$output .= '<tr>';
    	
    $ncols = $db->num_fields($stmt); 
    for ($i = 0; $i < $ncols; $i++) {
    	$column_name  = $db->field_name($stmt, $i); 
    	$output .= '<th>' . $column_name .  '</th>';
    }
    	
    $output .= "</tr>";
    	
    // 2. Creating individual HTML Table rows
	while ($current_row = $db->fetch_row($stmt)) { 
		
		$output .= '<tr>';
		
		for ($i = 0; $i < $ncols; $i++) {
			if ( ! is_object($current_row[$i])) {
       			$output .= '<td>' . $current_row[$i] . "</td>";
			}
			else {
				/*!
				 * \todo Using OCI_LOB->load() is a OCI specific hack, and does not use the generic dae_database API. This should be corrected in future releases
				 */
				try {
					$output .= '<td>' . $current_row[$i]->load() . "</td>";
				} catch (Exception $e) {
    				$output .= '<td> Caught exception: '.  $e->getMessage(). "</td>";
				}
			}
 			
    	}
    	
    	$output .= "</tr>";
	}

	// 3. Closing the HTML Table tags
	$output .= "</table>";
	$output .= "</p>";
		
	// 4. Cleaning up after we're done 
	oci_free_statement($stmt); // Can be generalized to mysql_fee_statement also.	
	
	return $output;
}

/**
 * \brief Form definition function for SQL query input
 * 
 * The form contains only two fields:
 *  - a \a textarea field named 'query'
 *  - a \a submit button named 'submit'
 *  
 * @param [in] $form_state
 * @return a Drupal form array.
 */
function data_repository_sql($form_state)
{
	/**
	 * Checks whether there is a previously known form entry for the query.
	 * If true, then pre-fill the form with this query. If not pre-fill the 
	 * form with a default query.
	 */
  if($_SESSION['data_repository']['formQuery'] == NULL)
  	$default_query = 'select * from PAGE_IMAGE';
  else
    $default_query = $_SESSION['data_repository']['formQuery'];
    
  $form = array();
  $form['query'] = array(
    '#type' => 'textarea',
    '#title' => t('SQL Query'),
    '#default_value' => $default_query,
    '#description' => t('SQL Query to submit to server')
  );
  $form['submit'] = array('#type' => 'submit', '#value' => t('Submit Query'));
  
  return $form;
}
/**
 * 
 * This function transforms URLs in the form of query/table/column/value into
 * select ID from TABLE where COLUMN like '%VALUE%';
 * 
 * @todo This function currently doesn't check whether ID exists in requested table
 */
function data_repository_queryurl()
{
    $variables = array();

    $db = getDAEDatabase();
    
    $variables[':urltable'] = strtoupper(arg(2));
    $tableName = $db->r("select object_name from all_objects where object_type = 'TABLE' and object_name = :urltable",$variables);
    $variables[':urlcolumn'] = strtoupper(arg(3));
    $columnName = $db->r("SELECT column_name FROM all_tab_cols where table_name = :urltable and column_name = :urlcolumn",$variables);

    if($tableName && $columnName) {

        unset($variables);
        $variables[':urlvalue'] = '%'.arg(4).'%';
    
        $queryReturn = $db->q("select ID from $tableName where $columnName like :urlvalue",$variables);
        while ($row = $db->fetch_array($queryReturn)) {
            print $row[0] . "\n";
        }
    }    
}

require_once("./".drupal_get_path('module','dae_data')."/dae_data_person.inc");

/**
 *
 * This function returns the DAE Database userId for the currently connected user
 * or returns an empty page otherwise.
 */
function data_repository_getPID()
{
	global $user;
	if($user->uid > 0)
		print getPID($user->uid);
}


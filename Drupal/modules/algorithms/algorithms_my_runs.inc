<?php
/**
 * @file 
 * 
 * @author  Mike Kot <mdk312@lehigh.edu>
 * 
 * @version 1.0
 *
 * \brief Build a list of runs initiated by the authenticated user.
 *  
 * @section LICENSE ²
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
 

require_once("./".drupal_get_path('module','dae_data')."/dae_data_person.inc");

function algorithms_my_runs()
{
    
	global $runs, $user;
	
	require_once("./".drupal_get_path('module','dae_ui')."/dae_ui.module");
	require_once("./".drupal_get_path('module','algorithms')."/algorithms_status.var");
	
	$db = getDAEDatabase();
	$ui = new RunUI("dae-run");

	$results = $db->q("select NAME, ALGORITHM_RUN_ID, ALGORITHM_ID, START_TIME, END_TIME, ALGORITHM_RUN.STATUS as STAT, " . 
					$db->dateFormat('END_TIME','MON DD, YYYY') . " as ENDDAY, " . 
					$db->dateFormat('END_TIME','HH:MI:SS') . " as ENDTIME 
					from ALGORITHM_RUN join EXECUTES_RUN on ID = ALGORITHM_RUN_ID natural join ALGORITHM_RUN_OF join ALGORITHM on ALGORITHM.ID = ALGORITHM_ID 
					where PERSON_ID = ". getPID($user->uid) ." order by START_TIME DESC, END_TIME",array());
	

	while($row = $db->fetch_array($results)):

		if(!algorithms_run_deleted($row['ALGORITHM_RUN_ID'])):
			
			$input_file = algorithms_get_run_input_filename($row['ALGORITHM_RUN_ID']);
			
			$info['left'] = "<a href=\"javascript:show()\">".$row['NAME'].(empty($input_file)?"":" ($input_file)")."</a>";
			$info['children'] = "collap";
			$info['id'] = $row['ALGORITHM_RUN_ID'];
			$info['image'] = "files.png";
			$info['onRowClick'] = "showRunOutput(this,".$row['ALGORITHM_RUN_ID'].")";
			$info['right'] = $ui->getRunStatus($row);
			
			$ui->addRecord($info); //build run record
			
		endif;

	endwhile;
	
	return $ui->getOutput();
		
}
/**
 * \brief Checks to see if the user has "deleted" the run
 * 
 * This function requires the drupal MySQL database
 *
 * Returns 1 (true) if the run has been deleted
 *
 * @param $run the run id.
 */
function algorithms_run_deleted($run)
{	
	$check = db_query("SELECT deleted FROM {runs_deleted} WHERE run_id = ". $run);
	
	return db_result($check);
}
/**
 * \brief Looks first in the spool directory then in the run directory for an input file.
 * 
 * This function requires the global variables $runs and $spool to be set to the appropriate directories 
 * 
 * If no input file is found, an empty string is returned
 * 
 * @param $run the run id.
 */
function algorithms_get_run_input_filename($run) ///@todo Almost definitely not stable.
{
	global $spool, $runs;

	$db = getDAEDatabase();
	
	settype($run, 'integer');
	$db_input = $db->q("select PATH from ALGORITHM_RUN_INPUT join PAGE_IMAGE on DATA_ITEM_ID = ID where ALGORITHM_RUN_ID = " . $run,array());
	
	while($row = $db->fetch_array($db_input)):
	
		if(empty($input))
			$input = basename($row['PATH']);
		else
			$input .= ", ".basename($row['PATH']);
	
	endwhile;

	if(empty($input)):
		if(is_dir($spool . '/' . $run))
			$files = hd_scandir($spool . '/' . $run);
		else if(is_dir($runs .'/'. $run . '/'))
			$files = hd_scandir($runs .'/'. $run . '/');
		else
			$files = array();
		foreach($files as $file):
			if($file != "output" && $file != "database.txt")
				if(empty($input)) $input = $file;
				else $input .= ", ".$file;
		endforeach;
	endif;
	
	return $input;
	
}
/**
 * \brief Responds to AJAX calls from My Runs page for information on a given Run
 * 
 * This function requires the global variable $spool to be set to the appropriate directory and takes the
 * assumption that all negative statuses are failures.
 * 
 * If the run is complete and the files have not already been moved, the function moves files from spool
 * to the user's runs directory when it calls algorithm_run_info_done()
 * 
 */
function algorithms_get_run_info()
{
  
	global $user, $spool;
	
	require_once("./".drupal_get_path('module','algorithms')."/algorithms_status.var");
	
	$run = arg(2);
	
	if(!algorithms_valid_run($run))
		return "";

	$status = algorithms_run_get_status($run);
	
	if($status == RUN_RUNNING):
		algorithms_get_run_info_working($run);
	
	elseif($status == RUN_COMPLETE):
		algorithms_get_run_info_done($run);
	
	elseif($status < 0):
		algorithms_get_run_info_failed(-1*$failure);
	
	else:
		algorithms_get_run_info_working($run);
	
	endif;
}
/**
 * \brief Retreives a run's status from the DAE db
 * 
 * This function will only allow users to check on their own runs
 * 
 * @param $run the run id.
 */
function algorithms_run_get_status($run)
{
	global $user;
	
	$db = getDAEDatabase();

	$variables[':run_id'] = $run;
	$variables[':user_id'] = getPID($user->uid);
	$checkDone = $db->r('select STATUS from ALGORITHM_RUN join EXECUTES_RUN on ID = ALGORITHM_RUN_ID where PERSON_ID = :user_id and ALGORITHM_RUN.ID = :run_id',$variables);

	return $checkDone;
	
}
/**
 * \brief Returns information on a run failure.
 * 
 * The function assumes the unix error numbers are positive
 * 
 * @param $failure the (positive) status integer from the DAE database
 */
function algorithms_get_run_info_failed($failure)
{	
	switch($failure)
	{
	case 1:
		echo "This run failed for an unknown reason.";
		break;
	default:
		echo "Run failed.  Please verify input and try again." ;
		break;
	}
}
/**
 * \brief Returns information on a run in progress
 * 
 * This function requires the UI class and db class 
 * 
 * Information on runtime is gathered and returned
 * 
 * @param $run the run id.
 */
function algorithms_get_run_info_working($run)
{	
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
/**
 * \brief Gathers information on a finished run and moves files to runs from spool if necessary
 * 
 * This function requires the UI and DB classes
 * 
 * @param $run the run id.
 */
function algorithms_get_run_info_done($run)
{		
	if(run_on_database($run))
		$output = algorithms_get_database_run_info_done($run);
	else
		$output = algorithms_get_home_folder_run_info_done($run);
	
	echo $output;	
}
function run_on_database($run)
{
	$db = getDAEDatabase();
	settype($run, 'integer');
	return $db->r('select count(*) from ALGORITHM_RUN_INPUT where ALGORITHM_RUN_ID = '.$run, array());
	
}
function algorithms_get_database_run_info_done($run)
{
	algorithms_check_files_moved_to_runs($run);
	
	$db = getDAEDatabase();
	
	$output .= "<div class=\"algorithm_run_input_files\"><h4>Input Files:</h4> <ul>";
	
	settype($run, 'integer');
	$input = $db->q('select ID, PATH from ALGORITHM_RUN_INPUT join PAGE_IMAGE on DATA_ITEM_ID = ID  where ALGORITHM_RUN_ID = '. $run, array());
	
	while($row = $db->fetch_array($input)):

		$output .= "<li>" .l(basename($row['PATH']),"browse/dataitem/".$row['ID']) ."</li>";		
			
	endwhile;
	
	$output .= "</ul> </div>";
	
	$output .= "<div class=\"algorithm_run_output_files\"><h4>Output Files: </h4><ul>";

	$run_output = $db->q("select ID, PATH from ALGORITHM_RUN_OUTPUT join PAGE_IMAGE on DATA_ITEM_ID = PAGE_IMAGE.ID  where ALGORITHM_RUN_ID = " . $run, array());
	
	while($row = $db->fetch_array($run_output)):

		$output .= "<li>" .l(basename($row['PATH']),"browse/dataitem/".$row['ID']) ."</li>";		
			
	endwhile;

	$output .= "</ul> </div>";
	
	$elapsed = $db->runLength($run);
    
    
    // Encode json:
    $json = json_encode(algorithms_get_database_run_graph($run));
    $json = str_replace(' ','_', $json );
    $json = trim($json, '[');
    $json =trim($json, ']');



	$output .= "<div class=\"algorithm_run_stats\">
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
			."</div></div>
            
            <div class=\"inner-details-infovis\" id=\"inner-details-infovis".$run."\" >
          
         Click on a node to display related information.
            </div>
            
             <div id=\"infovis$run\" class=\"infovis\">
                                    			                        <a href=javascript:init('infovis".$run."','".htmlspecialchars($json)."') style=\"display:none;\" class=\"infovis_anchor\">graph</a>

           </div> 
           
           <div id=\"log\"></div>

            ";
			                        //<a href=javascript:init('infovis".$run."','".htmlspecialchars($json)."')>graph</a>

	return $output;

           

}
function algorithms_get_home_folder_run_info_done($run)
{
	global $user, $runs;
	
	$db = getDAEDatabase();
	
	algorithms_check_files_moved_to_runs($run);
	
	$output .= "<div class=\"algorithm_run_input_files\"><h4>Input Files:</h4> <ul>";
	
	$files = hd_scandir($runs . "/$run");
	
	foreach($files as $file):
	
		if($file != "output")
			$output .= "<li>"
			.l($file,"getFile/runs/$run/$file")
			."</li>";		
			
	endforeach;
	
	$output .= "</ul> </div>";
	
	$output .= "<div class=\"algorithm_run_output_files\"><h4>Output Files: </h4><ul>";
	
	$files = hd_scandir($runs . "/$run/output");

	foreach($files as $file):
	
		if($file != "output.zip")
			$output .= "<li>"
			.l($file,"getFile/runs/$run/output/$file")
			."</li>";

	endforeach;
	

	$output .= "</ul> </div>";

	$elapsed = $db->runLength($run);
	
	$output .= "<div class=\"algorithm_run_stats\">
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
			
	return $output;

}
function algorithms_check_files_moved_to_runs($run)
{
    /*
	global $spool;
	
	$db = getDAEDatabase();
	
	$input = $db->r("select count(*) from ALGORITHM_RUN_INPUT where ALGORITHM_RUN_ID = " . $run);
	
	if(!empty($input)): //if data from db was used, save output back to db and delete spool files
		algorithms_save_output_to_db($run);
		//home_directory_delete_runs_folders($run);
	
	elseif(file_exists($spool .'/'. $run)):
	
		require_once("./".drupal_get_path('module','home_directory')."/home_directory_run_folder.inc");
				
		home_directory_copy_to_runs($run);
		home_directory_delete_runs_folders($run);
		home_directory_create_run_zip($run);
	
	endif;
	*/
}
function algorithms_save_output_to_db($run)
{
    /*
	global $spool;
	$db = getDAEDatabase();
	
	if($db->r("select count(*) from ALGORITHM_RUN_OUTPUT WHERE ALGORITHM_RUN_ID = $run"))
		return;
		
	$algorithm = $db->r("SELECT ALGORITHM_ID FROM ALGORITHM_RUN_OF WHERE ALGORITHM_RUN_ID = $run");
	
	$output = $db->q("select algo.NAME, d.ID as outputID, d.NAME as outputName, d.REQUIRED, d.ORDERING, dt.ID as datatype_id, dt.NAME as outputTypeName 
					from ALGORITHM algo, ALGORITHM_OUTPUT output, DATA d, TYPE_OF t, DATATYPE dt 
					where algo.ID=$algorithm and algo.ID=output.ALGORITHM_ID and output.DATA_ID=d.ID and d.ID=t.DATA_ID and t.DATATYPE_ID=dt.ID");
	
	$description = "Output from $algorithm";
	
	while($row = $db->fetch_array($output)):
		
		$files = hd_scandir($spool . "/$run/output/");
		
		$path = $spool . "/$run/output/".$files[key($files)];
		
		$db->i("insert into DATA_ITEM (ID, STATUS, DESCRIPTION, COPYRIGHT) values (seq_data_item.nextval, 0, '$description', '')");
		$dataitem = $db->r("SELECT seq_data_item.CURRVAL FROM DUAL");
		$db->i("insert into IMAGE_DATA_ITEM (ID) values ($dataitem)");
		$db->i("insert into PHYSICAL_IMAGE_DATA_ITEM (id) values ($dataitem)");
		$db->i("insert into PAGE_IMAGE (ID, PATH) values ($dataitem, '$path')");
		$db->i("insert into ALGORITHM_RUN_OUTPUT (ALGORITHM_RUN_ID, DATA_ITEM_ID) values ($run, $dataitem)");
		$db->i("insert into ASSOCIATE_DATATYPE_DATA_ITEM (DATA_ITEM_ID, DATATYPE_ID) values ($dataitem, ".$row['DATATYPE_ID'].")");
		$db->i("insert into INSTANCE_OF (data_item_id, data_id) values ($dataitem, ".$row['OUTPUTID'].")");
		
	endwhile;
	*/
}

function algorithms_get_database_run_graph($run)
{     
     
    $data= array();
    
    //Create the basic array of the execution graph
    $data=algorithms_fill_array_tree_right($data,$run,null,$run);
    $data=algorithms_fill_array_tree_left($data,$run,null,$run);

    
    
   /*
   Below is an example of the kind of array created. The rest of the fonction transform this array in an array representing the
   same graph, but unsing nested arrays to represent children-parent relationship, as in the JSON format.
   
   $data = array(
    array('id' => "1$run", 'parent_id' => null, 'name' => 'lorem ipsum'),
    array('id' => "2$run", 'parent_id' => "1$run", 'name' => 'lorem ipsum1'),
    array('id' => "3$run", 'parent_id' => "1$run", 'name' => 'lorem ipsum2'),
    array('id' => "4$run", 'parent_id' => "2$run", 'name' => 'lorem ipsum3'),
    array('id' => "5$run", 'parent_id' => "3$run", 'name' => 'lorem ipsum4'),
    );*/


$itemsByReference = array();

// Build array of item references:
foreach($data as $key => &$item) {
   $itemsByReference[$item['id']] = &$item;
   // Empty data class (so that json_encode adds "data: {}" ) 
   $itemsByReference[$item['id']]['data'] = new StdClass();
   //add info in "data: {}"
   $itemsByReference[$item['id']]['data']->output = $itemsByReference[$item['id']]['output'];
    $itemsByReference[$item['id']]['data']->input = $itemsByReference[$item['id']]['input'];

   $itemsByReference[$item['id']]['data']->run = $itemsByReference[$item['id']]['run'];
   // Children array:
   $itemsByReference[$item['id']]['children'] = array();
   
}

// Set items as children of the relevant parent item.
foreach($data as $key => &$item)
   if($item['parent_id'] && isset($itemsByReference[$item['parent_id']]))
      $itemsByReference [$item['parent_id']]['children'][] = &$item;

// Remove items that were added to parents elsewhere:
foreach($data as $key => &$item) {
   if($item['parent_id'] && isset($itemsByReference[$item['parent_id']]))
      unset($data[$key]);
}



return $data;
}

//This function adds all the nodes corresponding to executions realised after the root execution, including the root node.

function algorithms_fill_array_tree_right($array,$run,$parent,$rootrun)
    {
        
        //variable used to choose the name of nodes in the recursive fonctions 
        static $depth = 0;

        algorithms_check_files_moved_to_runs($run);
	
        $db = getDAEDatabase();
	
        settype($run, 'integer');
        
        // request for name algorithm
        $input = $db->q(
            'SELECT DISTINCT alg.name, dit_out.id,dit_in.id as ID2


            FROM algorithm_run arun   

           join algorithm_run_of arof on
           arof.algorithm_run_id = arun.id

           join algorithm alg on
           alg.id = arof.algorithm_id
           
            join algorithm_run_output arout on
           arout.algorithm_run_id = arun.id
    
           join data_item dit_out on
           dit_out.id = arout.data_item_id
           
            join algorithm_run_input arin on
           arin.algorithm_run_id = arun.id
    
           join data_item dit_in on
           dit_in.id = arin.data_item_id
 
           
            where arun.id ='.$run, array());
  
                // request for children nodes
       $input2 = $db->q(
            'SELECT distinct arun2.id

            FROM algorithm_run arun   

           join algorithm_run_of arof on
           arof.algorithm_run_id = arun.id

           join algorithm alg on
           alg.id = arof.algorithm_id

           join algorithm_run_output arout on
           arout.algorithm_run_id = arun.id
    
           join data_item dit_out on
           dit_out.id = arout.data_item_id
 

           join algorithm_run_input arin on
           arin.data_item_id = dit_out.id
  
           join algorithm_run arun2 on
           arun2.id = arin.algorithm_run_id

            join algorithm_run_of arof2 on
           arof2.algorithm_run_id = arun2.id

           join algorithm alg2 on
           alg2.id = arof2.algorithm_id

     
           where arun.id ='.$run, array());
           
            $idtemp=$rootrun;
            $row = $db->fetch_array($input);

            
            if($depth== 0){
                //case of root node
            $array[]=array('id' => $idtemp, 'parent_id' => null, 'name' => $row['NAME'], 'output' => $row['ID'],'input' => $row['ID2'], 'run' => $run); 

      }
      else{
                      $array[]=array('id' => $idtemp, 'parent_id' => $parent, 'name' => $row['NAME'], 'output' => $row['ID'],'input' => $row['ID2'], 'run' => $run); 

          }
      $idparent=$rootrun;
while($row2 = $db->fetch_array($input2)){
      $depth=$depth+1;
      // naming convention: "idroot => idnext => idnext" It guaranties unicity of the id among all nodes that coud ever be created
      $rootrun="$rootrun=>".$row2['ID'];
      $array=algorithms_fill_array_tree_right($array,$row2['ID'],$idparent,$rootrun);
      $depth=$depth-1;

       
}    
       if($depth==0){
//suppressing the static variable once it's no longer called recursively
$depth=null;
unset($depth);
} 
       return $array;
}



function algorithms_fill_array_tree_left($array,$run,$parent,$rootrun)
    {
        
        //variable used to choose the name of nodes in the recursive fonctions 

        static $depth = 0;

        algorithms_check_files_moved_to_runs($run);
	
        $db = getDAEDatabase();
	
        settype($run, 'integer');
        
        
        // request for name algorithm
        $input = $db->q(
            'SELECT DISTINCT alg.name,dit_in.id,dit_out.id as ID2
            FROM algorithm_run arun   

           join algorithm_run_of arof on
           arof.algorithm_run_id = arun.id

           join algorithm alg on
           alg.id = arof.algorithm_id
           
           join algorithm_run_input arin on
           arin.algorithm_run_id = arun.id

    
           join data_item dit_in on
           arin.data_item_id = dit_in.id
           
          join algorithm_run_output arout on
           arout.algorithm_run_id = arun.id
    
           join data_item dit_out on
           dit_out.id = arout.data_item_id
           
       
           
            where arun.id ='.$run, array());
  
                        
            // request for children node
            $input2 = $db->q(
            'SELECT distinct arun2.id

            FROM algorithm_run arun   

           join algorithm_run_of arof on
           arof.algorithm_run_id = arun.id

           join algorithm alg on
           alg.id = arof.algorithm_id
           
           
           join algorithm_run_input arin on
            arin.algorithm_run_id = arun.id

    
           join data_item dit_in on
           arin.data_item_id = dit_in.id

            join algorithm_run_output arout on
           arout.data_item_id = dit_in.id

  
           join algorithm_run arun2 on
           arun2.id = arout.algorithm_run_id

            join algorithm_run_of arof2 on
           arof2.algorithm_run_id = arun2.id

           join algorithm alg2 on
           alg2.id = arof2.algorithm_id

     
           where arun.id ='.$run, array());
           
            $idtemp=$rootrun;
            $row = $db->fetch_array($input);

            
            if($depth== 0){
      }
      else{
                      $array[]=array('id' => $idtemp, 'parent_id' => $parent, 'name' => $row['NAME'],'input' => $row['ID'],'output' => $row['ID2'], 'run' => $run); 

      }
      

      $idparent=$rootrun;
while($row2 = $db->fetch_array($input2)){
      $rootrun=$row2['ID']."<=$rootrun";
      $depth=$depth+1;
      $array=algorithms_fill_array_tree_left($array,$row2['ID'],$idparent,$rootrun);
       $depth=$depth-1;
}    

      if($depth==0){
//suppressing the static variable once it's no longer called recursively
$depth=null;
unset($depth);
} 
       
       return $array;
}




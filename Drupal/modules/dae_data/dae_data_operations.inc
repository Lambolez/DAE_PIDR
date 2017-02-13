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
 
 function dae_data_copyright()	//added by yang
{
	$id = arg(3);
	global $user;
	$db = getDAEDatabase();
	settype($id, 'integer');
	$path = $db->r("select PATH from FILES where ID =". $id,array());
//	drupal_set_message($path,"warning");
	$content = dae_data_mime($path);
	//header('Content-Disposition: attachment; filename='. basename($path));
	header("content-type: $content");
//	header("Content-Length: ".filesize($path));
	readfile($path);

} 

/**
 * \brief The callback function to retrieve copyright information before download and copy.
 * 
 */
 function dae_data_download_before()	//added by yang
{
	$dataset = arg(3);
	settype($dataset, 'integer');
	
	global $user;
	if(!check_dataset_id($dataset))
		echo "";
	$db = getDAEDatabase();
	$all = $db->q("select NAME, FILE_ID from HAS_COPYRIGHT B, FILES C where B.FILE_ID=C.ID and  B.DATA_ITEM_ID =".$dataset,array());
	$textMsg = "";
	$count = 0;
	while($row = $db->fetch_array($all)):
		$curMsg = "<a onclick=\"window.open('?q=browse/dataitem/copyright/".$row[FILE_ID]."','jav', 'width=800,height=800,resizable=yes,scrollbars') \"><U>" .$result.$row[NAME]."</U/></a>";
		if ($count == 0)
			$textMsg = $curMsg;
		else
			$textMsg = $textMsg.", ".$curMsg;
		$count = $count + 1;
	endwhile;

	echo $textMsg;
} 

/**
 * 
 * \brief Callback function allowing to actually download specific data_items from the database server
 * 
 *  \todo This function should check (how ??) whether the user has cleared the copyright disclaimer.
 *  
 *  This function checks whether the current argument (based on the Drupal URL \a arg(3)) is indeed
 *  a \a data_item id. It currently allows downloading of the following \a data_item types:
 *   - PAGE_IMAGE
 *   - FILE
 *   - DATASET
 *   
 */
function dae_data_download()
{
	$dataID = arg(3);
	global $user;
	if(!check_dataset_id($dataID))
		return "";
	$db = getDAEDatabase();
	$type = $db->getDataItemType($dataID);
	
	dae_data_increment_activity($dataID,2);
	
	db_query("insert into {dae_dataitem_download} values(".$user->uid.",now(),$dataID)");
	
	switch($type)
	{
		case DATA_ITEM_PAGE_IMAGE:
			$path = dae_data_get_page_image_path($dataID);
			$content = dae_data_mime($path);
			header('Content-Disposition: attachment; filename='. basename($path));
			header("content-type: $content");
			header("Content-Length: ".filesize($path));
			readfile($path);
			break;
		case DATA_ITEM_FILE:
		    $path = dae_data_get_file_path($dataID);
			$content = dae_data_mime($path);
			header('Content-Disposition: attachment; filename='. basename($path));
			header("content-type: $content");
			header("Content-Length: ".filesize($path));
			readfile($path);
			break;
		case DATA_ITEM_DATASET:
			$paths = dae_data_get_array_of_paths($dataID);
			$zip = dae_data_zip($paths);
			drupal_goto($zip);
			break;
			
		
		default:
			drupal_set_message("Invalid data item.",'warning');
			watchdog("Bad Dataitem ID",t("There was a failed user attempted to download data item %d",array("%d" => $dataID)));
			return "";
			break;
	}
}

function dae_data_get_download_size($dataID)
{
	global $user;
	if(!check_dataset_id($dataID))
		return "";
	$db = getDAEDatabase();
	$type = $db->getDataItemType($dataID);
	
	switch($type)
	{
		case DATA_ITEM_PAGE_IMAGE:
			$path = dae_data_get_page_image_path($dataID);
			return filesize($path);
			break;
		case DATA_ITEM_FILE:
		    $path = dae_data_get_file_path($dataID);
			return filesize($path);
		    break;
		case DATA_ITEM_DATASET:
			$paths = dae_data_get_array_of_paths($dataID);
			$totalSize = 0;
			foreach($paths as $filepath) {
			    $totalSize += filesize($filepath);
			}
			return $totalSize;
			break;
					
		default:
			drupal_set_message("Invalid data item.",'warning');
			watchdog("Bad Dataitem ID",t("There was a failed user attempted to download data item %d",array("%d" => $dataID)));
			return "";
			break;
	}
}

function dae_data_flag()
{
	$dataset = $_POST['ds'];
	
	if(!check_dataset_id($dataset))
		return "";
	
	$comments = $_POST['comments'];
	
	watchdog("Dataset Flag",t("A user flagged dataset %dataset as having a problem with comments:<br/>\"%comments\"",array("%dataset" => $dataset,"%comments" => $comments)), array(), WATCHDOG_WARNING);
	
	drupal_set_message("Thank you.  We will review your concern shortly.","status");
	
	return dae_data_browse();

}
function dae_data_copy()
{
	$dataset = arg(3);
	global $user;
	
	if(!check_dataset_id($dataset))
		return "";
	
	dae_data_increment_activity($dataset,2);
	
	$db = getDAEDatabase();
	$type = $db->getDataItemType($dataset);
	db_query("insert into {dae_dataitem_download} values(".$user->uid.",now(),$dataset)");
	settype($dataset, 'integer');
	
	switch($type)
	{
		case DATA_ITEM_PAGE_IMAGE:
			$paths[] = $db->r("select PATH from PAGE_IMAGE where ID = ". $dataset,array());
			$directory = "";
			dae_data_copy_to_home_directory($paths,$directory);
			break;
		
		case DATA_ITEM_DATASET:
			$paths = dae_data_get_array_of_paths($dataset);	
			$directory = $db->r("select NAME from dataset where ID = ". $dataset,array());
			dae_data_copy_to_home_directory($paths,$directory);
			break;
			
		
		default:
			drupal_set_message("Invalid data item.",'warning');
			watchdog("Bad Dataitem ID",t("There was a failed user attempted to download data item %d",array("%d" => $dataset)));
			return "";
			break;
	}
}

function dae_data_copy_to_home_directory($paths,$directory)
{
	global $uploads;
	
	require_once("./".drupal_get_path('module','home_directory')."/home_directory.module");
	
	if(!empty($directory))
		$directory = home_directory_make_directory($directory,$uploads.'/',true);
	
	$prefix = find_prefix($paths);
	$prefixlength = strlen($prefix);
	
	while ($prefixlength && $prefix[$prefixlength-1] != '/') {
        	$prefix = substr($prefix, 0, -1);
        	$prefixlength--;
    	}
	
	$directory = '/'. $directory .'/';
	
	foreach($paths as $key => $path):
		$dbpath = $path;
		$paths[$key] = substr($path,$prefixlength);
		$dirs = explode("/",$paths[$key]);
		end($dirs);
		unset($dirs[key($dirs)]);
		$dirs = implode("/",$dirs);
		if(!is_dir($uploads.$directory.$dirs))
			shell_exec("mkdir -p $uploads$directory$dirs");
		shell_exec("cp $dbpath $uploads$directory$paths[$key]");
		home_directory_change_usage(filesize($uploads.$directory.$paths[$key]));
	endforeach;
	
	drupal_goto("uploads/view$directory");
	
}
function dae_data_get_page_image_path($dataitem)
{
    settype($dataitem, 'integer');
	$db = getDAEDatabase();
	return $db->r("select PATH from PAGE_IMAGE where ID = " . $dataitem,array());
}
function dae_data_get_file_path($dataitem)
{
	settype($dataitem, 'integer');
	$db = getDAEDatabase();
	return $db->r("select PATH from FILES where ID = " . $dataitem,array());
}
function dae_data_get_array_of_paths($dataset)
{
	$db = getDAEDatabase();

	$paths = array();
	$leaves = array();
	
	/*
	if($db->checkDataSetHasChildren($dataset)):
		dae_data_get_dataset_leaves($dataset,$leaves);
	else:
		$leaves = array($dataset);
	endif;
	
	foreach($leaves as $leaf):
		
		$result = $db->q("select PATH from INCLUDES_PAGE_IMAGE,PAGE_IMAGE where DATASET_ID=". $leaf ." and PAGE_IMAGE_ID=ID
			union
				select PATH from INCLUDES_FILE,FILES where DATASET_ID=". $leaf ." and FILE_ID=ID
		",array());
		//databaseDebugMessage("select PATH from INCLUDES_PAGE_IMAGE,PAGE_IMAGE where DATASET_ID=". $leaf ." and PAGE_IMAGE_ID=ID");

		$paths = dae_data_get_paths_from_result($result,$paths);
	
	endforeach;
	*/
	
	$result = $db->q("
		select PATH 
			from 
				INCLUDES_PAGE_IMAGE,PAGE_IMAGE 
			where 
					PAGE_IMAGE_ID=ID 
				and 
					DATASET_ID in (
						select distinct(associating_dataset_id) 
							from 
								associate_dataset
							start with 
								dataset_id = $dataset
							connect by 
								prior associating_dataset_id = dataset_id 
						union 
						select $dataset from dual)
		union
		select PATH 
			from 
				INCLUDES_FILE,FILES 
			where 
				FILE_ID=ID 
			and DATASET_ID in (
					select distinct(associating_dataset_id) 
						from 
							associate_dataset
						start with 
							dataset_id = $dataset
						connect by 
							prior associating_dataset_id = dataset_id 
					union 
					select $dataset from dual)",
	array());
	
	$paths = dae_data_get_paths_from_result($result,$paths);
	
	return $paths;
}
function dae_data_get_paths_from_result($result,$paths)
{
	$db = getDAEDatabase();
	
	while($row = $db->fetch_array($result)):
			$paths[] = $row['PATH']; 
	endwhile;
	
	return $paths;
}
function dae_data_get_dataset_leaves($dataset,&$sets)
{
    settype($dataset, 'integer');
	$db = getDAEDatabase();
	$result = $db->q("select distinct ID from DATASET,ASSOCIATE_DATASET where ASSOCIATING_DATASET_ID=ID and DATASET_ID=" . $dataset,array());
	
	while($row = $db->fetch_array($result)):
		if(!$db->checkDataSetHasChildren($row['ID'],$sets))
			$sets[] = $row['ID'];
		else
			dae_data_get_dataset_leaves($row['ID'],$sets);
	endwhile;
}
/**
*
*/

function find_prefix($array)
{
	$prefix = array_shift($array);  // Select the first item as initial prefix
	$length = strlen($prefix);

	foreach ($array as $currentitem) {
		
    	/**
    	*  Check if the current prefix is still prefix of the current element in the array
    	*  If not, find the longest common prefix by removing characters from the end.
    	**/
    	while ($length && substr($currentitem, 0, $length) !== $prefix) {
        	$prefix = substr($prefix, 0, -1);
        	$length--;
    	}
    	if (!$length) break;
    }	
    
    return $prefix; 
}

function dae_data_zip($paths)
{
	global $user;
	
	$prefix = find_prefix($paths);
	$prefixlength = strlen($prefix);
	
	while ($prefixlength && $prefix[$prefixlength-1] != '/') {
        	$prefix = substr($prefix, 0, -1);
        	$prefixlength--;
    	}
	
	$time = date("YmdHis");
	$filename = "/dae/spool/".$user->uid."/".$time.".tar";
	$fileList = "/dae/spool/".$user->uid."/".$time.".txt";
	
	if(! is_dir("/dae/spool/".$user->uid)) {
	    mkdir("/dae/spool/".$user->uid);
	}
	
	$fh = fopen($fileList, 'w') or die("Failed to open file $fileList");
    	
	foreach($paths as $key => $path):
		$itemPath = substr($path,$prefixlength) . "\n";
		fwrite($fh, $itemPath);	
	endforeach;
	
    fclose($fh);
	
	shell_exec("cd '$prefix' && tar cf '$filename' -I '$fileList' && rm '$fileList'");
	
	return "getFile/tmp/$time.tar";
}
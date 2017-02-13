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
 
require_once("./".drupal_get_path('module','dae_data')."/dae_data_operations.inc");

 /**
 * This function decides which data items to return based on the request url
 *
 * Adding a browse by case:
 *
 * Each browse by case specifies the different parts of all possibile and necessary queries
 * Since data can be filtered by page images, root datasets, etc the queries are built by specifying and
 * combining the select, from, group, where of a given type of data.  In other words, if a user filters by all, 
 * the a query is built from the 'PAGE_IMAGE' array and the 'DATASET' array, followed by a union of the 
 * two to get the result.  In the event that $queries['DATASET']['SELECT'] or some other part of the ['DATASET']
 * array does not exist, the corresponding ['ALL'] is used in its place.  Finally, ['ROW_NUMBERED'] specifies a
 * select that includes all information and a row number as RN.
 * 
 * The dataset page builder assumes the the data item id "DATA_ITEM", and the name to be displayed is "NAME"
 * 
 * Adding a new filter is as simple as adding any necessary query parts and adding an array with the corresponding 
 * name filter type to the filter switch.  Also remember to add the data type/filter to All
 *
 * 
 * @param arg(1) - a string which describes the group of data to display
 * @return - the html for a data browsing page
 */
function dae_data_browse()
{
	$db = getDAEDatabase();
	$filter = $_GET['filter']; //Only show datasets, root datasets, etc

	$pagination = dae_data_pagination(variable_get('dae_data_per_page', 10)); //gets requested per_page and page number or uses defaults
	extract($pagination);
	
	$browse_by = arg(1);
	$page_limit = "WHERE RN > ".($per*($page-1))." AND RN <= ".$per*$page;
	$variables = array();

	switch($browse_by) {
		case "top-rated":
			$queries['ALL']['SELECT']        = "SELECT DATA_ITEM, NAME, SUM(RATING)/COUNT(RATING) AS AVG_RATING, 'DATASET' AS TYPE";
			$queries['ALL']['FROM']          = "FROM DAE_DATAITEM_RATING JOIN DATASET ON DATA_ITEM = ID";
			$queries['ALL']['GROUP']         = "GROUP BY DATA_ITEM, NAME";
			$queries['ALL']['WHERE']         = "WHERE NAME IS NOT NULL";
			$queries['ALL']['ORDER']		 = "ORDER BY AVG_RATING DESC"; ///default clauses for queries
			$queries['PAGE_IMAGE']['SELECT'] = "SELECT DATA_ITEM, PATH AS NAME, SUM(RATING)/COUNT(RATING) AS AVG_RATING, 'PAGE_IMAGE' AS TYPE";
			$queries['PAGE_IMAGE']['FROM']   = "FROM DAE_DATAITEM_RATING JOIN PAGE_IMAGE ON DATA_ITEM = ID";
			$queries['PAGE_IMAGE']['WHERE']  = "WHERE PATH IS NOT NULL";
			$queries['PAGE_IMAGE']['GROUP']  = "GROUP BY DATA_ITEM, PATH"; ///clauses when querying for page images
			$queries['ROOTS']['WHERE']       = "WHERE ID NOT IN (SELECT ASSOCIATING_DATASET_ID FROM ASSOCIATE_DATASET) AND NAME IS NOT NULL"; //clause when querying root datasets
			$queries['ROW_NUMBERED']         = "SELECT DATA_ITEM, NAME, AVG_RATING, TYPE, ROW_NUMBER() OVER(ORDER BY AVG_RATING DESC) AS RN FROM(";
			break;
			
		case "popular": 
			
			$days = dae_data_popular_interval($default = 30); //get interval to measure over
			settype($days,'integer');
			
			$queries['ALL']['SELECT']        = "SELECT DATA_ITEM, NAME, SUM(ACTIVITY) AS TOTAL, 'DATASET' AS TYPE";
			$queries['ALL']['FROM']          = "FROM DAE_DATAITEM_POPULAR JOIN DATASET ON DATA_ITEM = ID";
			$queries['ALL']['GROUP']         = "GROUP BY DATA_ITEM, NAME";
			$queries['ALL']['WHERE']         = "WHERE ROUND(SYSDATE-POPULAR_DATE) < $days";
			$queries['ALL']['ORDER']		 = "ORDER BY TOTAL DESC";
			$queries['PAGE_IMAGE']['SELECT'] = "SELECT DATA_ITEM, PATH AS NAME, SUM(ACTIVITY) AS TOTAL, 'PAGE_IMAGE' AS TYPE";
			$queries['PAGE_IMAGE']['FROM']   = "FROM DAE_DATAITEM_POPULAR JOIN PAGE_IMAGE ON DATA_ITEM = ID";
			$queries['PAGE_IMAGE']['WHERE']  = "WHERE ROUND(SYSDATE-POPULAR_DATE) < $days AND PATH IS NOT NULL";
			$queries['PAGE_IMAGE']['GROUP']  = "GROUP BY DATA_ITEM, PATH";
			$queries['ROOTS']['WHERE']       = "WHERE ID NOT IN (SELECT ASSOCIATING_DATASET_ID FROM ASSOCIATE_DATASET) AND ROUND(SYSDATE-POPULAR_DATE) < $days AND NAME IS NOT NULL";
			$queries['ROW_NUMBERED']         = "SELECT DATA_ITEM, NAME, TOTAL, TYPE, ROW_NUMBER() OVER(ORDER BY TOTAL DESC) AS RN FROM(";
			
			break;
		case "tags":
			$tag = check_plain(arg(2));
			drupal_set_title(t("Browse Data with Tag %t", array("%t" => $tag)));
			$browse_by = "tags/$tag";
			
			$variables[':tag'] = $tag;

			$queries['ALL']['SELECT']        = "SELECT DATASET.ID AS DATA_ITEM, NAME, 'DATASET' AS TYPE";
			$queries['ALL']['FROM']          = "FROM TAG JOIN ASSOCIATE_DATA_ITEM_TAG ON TAG.ID = TAG_ID JOIN DATASET ON DATASET.ID = DATA_ITEM_ID";
			$queries['ALL']['GROUP']         = "";
			$queries['ALL']['WHERE']         = "WHERE VALUE = :tag AND NAME IS NOT NULL";
			$queries['ALL']['ORDER']		 = "ORDER BY NAME";
			$queries['PAGE_IMAGE']['SELECT'] = "SELECT DATA_ITEM_ID AS DATA_ITEM, PATH AS NAME, 'PAGE_IMAGE' AS TYPE";
			$queries['PAGE_IMAGE']['FROM']   = "FROM TAG JOIN ASSOCIATE_DATA_ITEM_TAG ON TAG.ID = TAG_ID JOIN PAGE_IMAGE ON DATA_ITEM_ID = PAGE_IMAGE.ID";
			$queries['PAGE_IMAGE']['GROUP']  = "";
			$queries['PAGE_IMAGE']['WHERE']  = "WHERE VALUE = :tag AND PATH IS NOT NULL";
			$queries['ROOTS']['WHERE']       = "WHERE VALUE = :tag AND ID NOT IN (SELECT ASSOCIATING_DATASET_ID FROM ASSOCIATE_DATASET) AND NAME IS NOT NULL";
			$queries['ROW_NUMBERED']         = "SELECT DATA_ITEM, NAME, TYPE, ROW_NUMBER() OVER(ORDER BY DATA_ITEM DESC) AS RN FROM(";
			break;
		default:
		case "new":
			$browse_by = "new";
			$queries['ALL']['SELECT']        = "SELECT ID AS DATA_ITEM, NAME, 'DATASET' AS TYPE";
			$queries['ALL']['FROM']          = "FROM DATASET";
			$queries['ALL']['GROUP']         = "";
			$queries['ALL']['WHERE']         = "WHERE NAME IS NOT NULL";
			$queries['ALL']['ORDER']		 = "ORDER BY DATA_ITEM DESC";
			$queries['PAGE_IMAGE']['SELECT'] = "SELECT ID AS DATA_ITEM, PATH AS NAME, 'PAGE_IMAGE' AS TYPE";
			$queries['PAGE_IMAGE']['FROM']   = "FROM PAGE_IMAGE";
			$queries['PAGE_IMAGE']['WHERE']  = "WHERE PATH IS NOT NULL";
			$queries['PAGE_IMAGE']['GROUP']  = "";
			$queries['ROOTS']['WHERE']       = "WHERE ID NOT IN (SELECT ASSOCIATING_DATASET_ID FROM ASSOCIATE_DATASET) AND NAME IS NOT NULL";
			$queries['ROW_NUMBERED']         = "SELECT DATA_ITEM, NAME, TYPE, ROW_NUMBER() OVER(ORDER BY DATA_ITEM DESC) AS RN FROM(";
			break;
	}
	
	switch($filter)
	{
		case 'roots':
			$types = array("ROOTS");
			break;
		case 'datasets':
			$types = array("DATASET");
			break;
		case 'page_images':
			$types = array("PAGE_IMAGE");
			break;
		case 'all':
			$types = array("DATASET","PAGE_IMAGE");
			break;
		default:
			if($browse_by == "new")
				$types = array("ROOTS");
			else
				$types = array("DATASET","PAGE_IMAGE"); //List of queries to build
			break;
	}
	
	foreach($types as $type):
		$query[] .= (!empty($queries[$type]['SELECT'])?$queries[$type]['SELECT']:$queries['ALL']['SELECT'])." ".
				    (!empty($queries[$type]['FROM'])?$queries[$type]['FROM']:$queries['ALL']['FROM'])." ".
				    (!empty($queries[$type]['WHERE'])?$queries[$type]['WHERE']:$queries['ALL']['WHERE'])." ".
				    (!empty($queries[$type]['GROUP'])?$queries[$type]['GROUP']:$queries['ALL']['GROUP'])." ";
	endforeach;
	
	$query = implode("UNION ",$query) . $queries['ALL']['ORDER']; //turn array of queries into a single unioned query
	
	/** \todo Secure following two queries from SQL injection **/
	$count = $db->r("SELECT COUNT(*) FROM (" . $query . ")",$variables); //build a query that counts the unioned array for the total number of items
	
	$paged_query = "SELECT * FROM (" . $queries['ROW_NUMBERED'] . $query . ") )" . $page_limit; //select only the items for the given page
	
	$pagination = array('count' => $count, 'per' => $per, 'page' => $page, 'browse_by' => $browse_by."/".arg(2));
	
	$results[] = $db->q($paged_query,$variables);
	
	return dae_data_display_datasets($results,$db,$set['ordered_array'],$pagination);
}
/**
 * This function takes a Drupal MySQL result with column 'data_item'
 * and returns an array containing a comma separated string of the data item IDs
 * and an array of the IDs in the correct display order
 *  
 * @param $result - a MySQL result which has data_item as a column name
 * @return - an array containing a comma separated string of the data item IDs
 * 			and an array of the IDs in the correct display order 
 */
function dae_data_get_drupal_db_list($result)
{
	$set = array();
	while($row = db_fetch_array($result)):
		$set[] = $row['data_item'];
	endwhile;
	$output = array('ordered_array' => $set,'query_text' => implode(",",$set));
	if(empty($output['query_text']))
		$output['query_text'] = 0;
	return $output;
}
/**
 * This function interprets pagination variables in the request URL
 * It assumes the variables per_page and page contain the pagination information.
 * It does not allow more than 100 data items per page.
 *  
 * @param $default_limit - if no per_page is specified, how many items to show per page
 *
 * @return - an array containing 'per' with the number of items per page and 'page' containing
 *			the number of the page to be viewed.
 */
function dae_data_pagination($default_limit = 10)
{
	$page = $_GET['page'];
	$per = $_GET['per_page'];
	
	if(is_numeric($page) && is_numeric($per) && $page > 0 && $per > 0 && $per < 101)
		return array('per' => $per,'page' => $page);
	elseif(is_numeric($per) && $per > 0 && $per < 101)
		return array('per' => $per,'page' => 1);
	elseif(is_numeric($page) && $page > 0)
		return array('per' => $default_limit,'page' => $page);
	else
		return array('per' => $default_limit, 'page' => 1);
}
/**
 * This function interprets popular intervals and returns the correct number of days
 *  
 * @param $default - if no default is specified, how far back to check for popular
 *
 * @return - the number of days to scan over
 */
function dae_data_popular_interval($default = 30)
{
	$interval = arg(2);
	switch($interval)
	{
		case "today":
			return 1;
			break;
		case "week":
			return 7;
			break;
		case "month":
			return 30;
			break;
		case "year":
			return 365;
			break;
		case "all-time":
			return 1000000;
			break;
		default:
			return $default;
			break;
	}
}

/**
 * This function is used to bridge the gap between the Drupal and Oracle Databases.
 * Using the filter and a list of data items, it selected all the data items needed for a given page.
 * If the filter is not recognized, all types of data items are returned.
 *  
 * @param $data_items   a comma separated string list of data items to select.  IDs may refer to any
 * 						type of data item
 * @param $filter  the requested filter for the page (type of data item).
 *
 * @return an array of oracle results that contain the data items requested in the $data_items list
 */
function dae_data_browse_get_results($data_items,$filter)
{
	$db = getDAEDatabase();
	
	$variables[':dataitems'] = $data_items;
	
	switch($filter)
	{
		case "datasets":
			$results[] = $db->q('select distinct NAME,ID from DATASET where ID in(:dataitems) and NAME is not null', $variables);
			break;
		case "roots":
			$results[] = $db->q('select distinct NAME,ID from DATASET where ID not in (select ASSOCIATING_DATASET_ID from ASSOCIATE_DATASET) and ID in (:dataitems)  and NAME is not null', $variables);
			break;
		case "page_images":
			$results[] = $db->q('select distinct PATH,ID from PAGE_IMAGE where ID in(:dataitems) and PATH is not null', $variables);
			break;
		default:
			$results[] = $db->q('select distinct NAME,ID from DATASET where ID in(:dataitems) and NAME is not null', $variables);
			$results[] = $db->q('select distinct PATH,ID from PAGE_IMAGE where ID in(:dataitems) and PATH is not null', $variables);
			break;
	}
	return $results;
}

function human_readable($size) {
 
    $newsize = $size;
    
    $units = array('B','KB','MB','GB','TB');
    for ($i = 0; $newsize > 1024; $i++) {
        $newsize /= 1024;
    }
    
    return round($newsize) . $units[$i];
}

/**
 * Takes an array of results and an order of data items to build the data items page
 *  
 * @param $results - an array of oracle results with the columns ID and NAME or ID and PATH
 * @param $db - a DAE database object
 * @param $order - an array of data item IDs in the order they are to be displayed
 * @param $pagination - an array containing the per page, page and total number of rows information
 *
 * @return html to be displayed
 */
function dae_data_display_datasets(&$results,&$db,$order = array(),&$pagination = array())
{
	$i = 0;
	require_once(drupal_get_path('module','dae_data')."/dae_data_search.inc");
	//$ui = new DataUI(drupal_get_form("dae_data_search_page"),$pagination);
	
	//added by yil308 for dropdown list.
	$ui = new DataUI(drupal_get_form("dae_data_search_page_select"),$pagination);
	
	foreach($results as $result):
	
		while($row = $db->fetch_array($result)):
			
			if(empty($row['VALUE'])){
        $id = $row['DATA_ITEM'];
        $name = $row['NAME'];
        $activity = number_format(dae_data_get_activity($id,dae_data_popular_interval()));
				if($row['TYPE'] == "DATASET"):
          $s = new StarRating($id,dae_data_get_rating($id,false),false,false);
          $stars = $s->getOutput();
          $tags = dae_data_get_tags($id);
					$info['output'] =
            "<li class=\"data-home-list-item\" id=\"".$id."\">
              <div class=\"dae-data-browse-thumb\">
				        <div class=\"dae-data-browse-thumb-align\">".
					        l("<img src=\"".drupal_get_path("module","dae_ui").
                    "/images/dae_data/dataset-thumb.jpg\" alt=\"\" />","browse/dataitem/".$id,array('html' => true))."
				        </div>
			        </div>
			        <h3>".l($name, "browse/dataitem/".$id)."</h3><br>"
              .dae_dataset_preview($id).
			          $stars.
			        "<div class=\"dae-data-activity\">".
				        $activity
			        ."</div>
			        <div class=\"dae-data-tags\">
				        <h6>TAGS: </h6>".
				        $tags
			        ."</div>".
		        "</li>";
			  elseif($row['TYPE'] == "PAGE_IMAGE"):
          $info['output'] =
          "<div id=\"dataitem-".$id."\" class=\"dataset-item single-item\">".
            "<div class=\"item-container\">".
              "<a class=\"item-link\" href=\"".base_path()."?q=browse/dataitem/".$id."\">
              <img src=\"".dae_data_get_thumb_uri($id, 'medium')."\" alt=\"\" />
              </a>
            <span id=\"name-".$id."\" title=\"".basename($name)."\" class=\"item-title\">".basename($name)."</span>".
            "</div>".
            "<div class=\"item-info\">".
              "<img src=\"".drupal_get_path("module","dae_ui")."/images/ajax.gif\">".
            "</div>".
          "</div>";
				endif;
				$info['id'] = $id;
				$info['tags'] = $tags;
				$info['activity'] = $activity;
				$info['rating'] = dae_data_get_rating($row['DATA_ITEM'],false);
				$info['stars'] = $stars;
				
				$ui->addDataitemRecord($info);
			}
			else{
				$info['name'] = l($row['VALUE'],"browse/tags/".$row['VALUE']);
				$info['id'] = $row['VALUE'];
				$info['thumb_source'] = drupal_get_path("module","dae_ui")."/images/dae_data/tag-thumb.jpg";
				$ui->addTagRecord($info);
			}

			unset($info);
		
		endwhile;

	endforeach;

	return $ui->getOutput($order);
}
/**
 * This function gets the activity rating of a data item from the Drupal Database
 *  
 * @param [in] $dataitem  an ID of a data item to retrieve activity rating for
 * @param [in] $days
 *
 * @return an integer representing the data item's activity rating
 */
function dae_data_get_activity($dataitem,$days)
{
	$db = getDAEDatabase();
	
	settype($dataitem, 'integer');
	settype($days, 'integer');
	
	$q = $db->q("SELECT sum(ACTIVITY) A 
				 FROM DAE_DATAITEM_POPULAR
				 WHERE ROUND(SYSDATE-POPULAR_DATE) < $days AND DATA_ITEM = $dataitem 
				 GROUP BY DATA_ITEM",array());
	$act = $db->fetch_array($q);
	if(empty($act['A']))
		return "0";
	else
		return $act['A'];
	
}
/**
 * This function is used get a comma separated list of tag links 
 *  
 * @param $dataitem the ID of a dataitem to get tag list for
 *
 * @return a comma separated list of tag links
 */
function dae_data_get_tags($dataitem)
{
		$db = getDAEDatabase();
		settype($dataitem, 'integer');
		$tags = $db->q("select VALUE from TAG join ASSOCIATE_DATA_ITEM_TAG on TAG_ID = ID where DATA_ITEM_ID = $dataitem",array());
		
		while($tagrow = $db->fetch_array($tags)):
			$taglist[] = l($tagrow['VALUE'],"browse/tags/".$tagrow['VALUE']);
		endwhile;
		
		if(!count($taglist))
			$taglist = array('No Tags Yet');
		
		return implode(", ",$taglist);
}
/**
 * Depricated.  Marked for deletion. 
 *
 * @todo remove function and dependencies
 *
 */
function dae_data_browse_subset()
{
	$db = getDAEDatabase();
	$ui = new DataUI("dae_data");
	
	$subset = arg(2);
	
	if(!check_dataset_id($subset))
		return "";
		
	settype($subset, 'integer');
	$result = $db->q("select distinct NAME,ID from DATASET,ASSOCIATE_DATASET where ASSOCIATING_DATASET_ID=ID and DATASET_ID=" . $subset,array());
	
	echo dae_data_display_datasets($result,$ui,$db,$color);

}
/**
 * This function looks for a search string GET 's' and returns matching tags and data items 
 *
 * @return html to build a search results page
 */
function dae_data_search_results_page()
{
	$search = check_plain($_GET['s']);
	
	drupal_set_title(t("Search Results for %s",array('%s' => $search)));
	drupal_add_js("var search = '$search';",'inline');
	$db = getDAEDatabase();
	if($_GET['page'] <= 1)
		$results[] = $db->searchTable('result',$search,"VALUE",array("ID AS DATA_ITEM", "VALUE", "'TAG' AS TYPE"),"TAG",0,0);
	$count = $db->searchTable('count',$search,"NAME",array("ID AS DATA_ITEM", "NAME", "'TAG' AS TYPE"),"DATASET",0,0,array('start' => true, 'end' => true));
	$pagination = dae_data_pagination(variable_get('dae_data_per_page', 10));
	extract($pagination);
	$pagination = array('count' => $count, 'per' => $per, 'page' => $page, 'browse_by' => check_plain(arg(1)."/".arg(2)));
	$results[] = $db->searchTable('result',$search,"NAME",array("ID AS DATA_ITEM", "NAME", "'DATASET' AS TYPE"),"DATASET",($page-1)*$per,$page*$per,array('start' => true, 'end' => true));
	return dae_data_display_datasets($results,$db,null,$pagination);
	
}
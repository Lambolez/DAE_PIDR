<?php
// $Id:$

/**
 * @file dae_data_detail_page.inc
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

 $db = getDAEDatabase();
 global $db;

/**
 * This function is called when browse/dataitem is requested.  It takes the
 * data item id from argument(2) and attempts to find the correct function to
 * make a page for the type of data item
 *  
 * @param arg(2) - the data item id
 *
 * @return html for a data item page
 */
function dae_data_browse_dataitem()
{
	global $user, $db;
	$dataitem = arg(2);
  $page = arg(3);
  if(!isset($page)) $page =1;
	
	if(!check_dataset_id($dataitem)) //confirm queried dataset is just a number
		return "";
	
	dae_data_increment_activity($dataitem,1); //add to popularity rating
	
	switch($db->getDataItemType($dataitem))
	{
		case DATA_ITEM_PAGE_IMAGE:
			return dae_pageimage_page(dae_data_get_pageimage_info_array($dataitem));
			break;
		case DATA_ITEM_PAGE_ELEMENT:
			return dae_page_element_page(dae_data_get_page_element_info_array($dataitem));
			break;
		case DATA_ITEM_DATASET:
			return dae_dataset_page(dae_data_get_dataset_info_array($dataitem), $page);
			break;
		case DATA_ITEM_FILE:
			return dae_file_page(dae_data_get_file_info_array($dataitem));
			break;
		default:
			drupal_set_message("Data item does not exist",'warning'); //give up
			return "";
			break;
	}		
}
/**
 * This function is called when a data item is determined to be a dataset
 *
 *  
 * @param $request - an array containing ['NAME'] = the display name of the data item and
 *					 ['ID'] = the dataset id
 *
 * @return html to display a dataset page
 */
function dae_dataset_page($request, $limit = 1)
{	
	$db = getDAEDatabase();
	
	drupal_set_title(t("!name",array('!name' => $request['NAME']))); //update page name to dataset name
	
	$rating = dae_data_get_rating($request['ID']); //get dataset rating

	$info = array('id' => $request['ID'], //set various dataset information for UI class
				  'comments' => get_dataitem_comments($request['ID']), //formatted html of the comments on this data item
				  'commentForm' => get_dataitem_comment_form($request['ID']), //a form to add a comment to the data item
				  'toolbar' => array('tag_form' => get_dae_data_tag_form()), //a form to add a tag to the data item
				  'title' => array(
				  			'name' => $request['NAME'], //the display name for the data item
				  			'submitter' => get_dataitem_submitter($request['ID']),
				  			'stars' => array(
				  					   'id' => $request['ID'], //id of the data item
				  					   'rating' => $rating['rating'], //average rating, or logged in user's rating
				  					   'rated' => $rating['rated'] //0 signifies average rating, 1 the logged in user's rating
				  					   )
				  			)
		    );

	$sets = array();
	$dataset_names = array();
	
	dae_data_get_dataset_hierarchy($request['ID'],$sets,$dataset_names); //build arrays of datasets, their names, and their relationships
	
	$info['set_array'] = $sets; //add to the UI class information array
	$info['set_names'] = $dataset_names;
	
	$pui = new PageImageList($info); //UI class for list of data items
	//if(dae_data_build_page_image_list($info['id'], $pui))
	
	dae_data_build_page_image_list($request['ID'], $pui, $limit);
	
	$info['page_images'] = $pui->getOutput();
	
	//else
	//	$info['page_images'] = "";
	
	$ui = new DatasetPage($info); //build page HTML
	
  $js = 'selectedItem = '.$request['ID'].';
        pageType =\'dataset\';';
  drupal_add_js($js, inline);
  
	return $ui->getOutput();
}
/**
 * This function is called when a data item is determined to be a page image
 *
 *  
 * @param [in] $request an array containing ['PATH'] = the path to be turned into the display name of the data item and
 *					 ['ID'] = the page image id
 *
 * @return html to display a page image page
 */
function dae_pageimage_page($request)
{
	$db = getDAEDatabase();
	
	require_once(drupal_get_path('module','dae_data')."/dae_data_page_elements.inc");
	
	$image = "?q=browse/dataitem/thumb/".$request['ID']."/large"; //large thumbnail path
	
	$imagesize = getimagesize($request['PATH']); //size of the actual image

	$js = 'selectedItem = '.$request['ID'].';
		   pageimage = true;
		   tci = selectedItem;
		   loadTree();
		   var truesize = ['.$imagesize[0].','.$imagesize[1].'];
		   '; //prep javascript with the data item id and the actual size of the image
		   
	drupal_add_js(drupal_get_path('module','dae_ui')."/javascript/jquery.Jcrop.js"); //js for page element creation
	drupal_add_js(drupal_get_path('module','dae_ui')."/javascript/jquery.Jcrop.min.js"); //js for page element creation
	drupal_add_js($js,'inline');
	//drupal_add_js("var page_elements = ".get_page_element_boxes($request['ID']),'inline'); //json for page element boxes
	
	drupal_set_title(basename($request['PATH']));
	
	$rating = dae_data_get_rating($request['ID']);

	$info = array(
			'id'                =>  $request['ID'],
			'path'              =>  $image,
			'comments'          =>  get_dataitem_comments($request['ID']),
			'commentForm'       =>  get_dataitem_comment_form($request['ID']),
			'page_element_form' =>  drupal_get_form('dae_data_page_element_form'),
			'toolbar'           =>  array('tag_form' => get_dae_data_tag_form()),
			'title'             =>  array(
									'name'      => basename($request['PATH']), 
									'submitter' => get_dataitem_submitter($request['ID']), 
									'stars'     => array(
													'id'     => $request['ID'],
							 						'rating' => $rating['rating'], 
							 						'rated'  => $rating['rated']
							 						)
									)
			);
	
	
	
	$ui = new PageImagePage($info);
	
	return $ui->getOutput();
}

/**
 * This function is called when a data item is determined to be a file
 *
 *  
 * @param [in] $request an array containing ['PATH'] = the path to be turned into the display name of the data item and
 *					 ['ID'] = the page image id
 *
 * @return html to display a page image page
 */
function dae_file_page($request)
{
	$db = getDAEDatabase();
	
	require_once(drupal_get_path('module','dae_data')."/dae_data_page_elements.inc");
	
	$js = 'selectedItem = '.$request['ID'].';
		   pageimage = true;
		   tci = selectedItem;
		   loadTree();
		   '; //prep javascript with the data item id and the actual size of the image
		   
	drupal_add_js(drupal_get_path('module','dae_ui')."/javascript/jquery.Jcrop.js"); //js for page element creation
	drupal_add_js(drupal_get_path('module','dae_ui')."/javascript/jquery.Jcrop.min.js"); //js for page element creation
	drupal_add_js($js,'inline');
	//drupal_add_js("var page_elements = ".get_page_element_boxes($request['ID']),'inline'); //json for page element boxes
	
	drupal_set_title(basename($request['PATH']));
	
	$rating = dae_data_get_rating($request['ID']);

	$info = array(
			'id'                =>  $request['ID'],
			'path'              =>  $request['PATH'],
			'comments'          =>  get_dataitem_comments($request['ID']),
			'commentForm'       =>  get_dataitem_comment_form($request['ID']),
			'toolbar'           =>  array('tag_form' => get_dae_data_tag_form()),
			'title'             =>  array(
									'name'      => basename($request['PATH']), 
									'submitter' => get_dataitem_submitter($request['ID']), 
									'stars'     => array(
													'id'     => $request['ID'],
							 						'rating' => $rating['rating'], 
							 						'rated'  => $rating['rated']
							 						)
									)
			);
	
	
	
	$ui = new FilePage($info);
	
	return $ui->getOutput();
}

/**
 * This function is called when a data item is determined to be a page element
 *
 * @param [in] $request an array containing ['PARENT_PAGE_IMAGE_ID'] = the id of the parent page image
 *					 ['PARENT_PAGE_IMAGE_PATH'] = the path of the parent page image
 *					 ['ID'] = the id of the page element
 *					 ['X'] = top left x position of element
 *					 ['Y'] = top left y position of the element
 *					 ['WIDTH'] = width of element
 *					 ['HEIGHT'] = height of element
 *
 * @return html to display a page element page
 */
function dae_page_element_page($request)
{
	$db = getDAEDatabase();
	
	require_once(drupal_get_path('module','dae_data')."/dae_data_page_elements.inc");		
	
	$image = "?q=browse/dataitem/thumb/".$request['PARENT_PAGE_IMAGE_ID']."/large";
	$pixel_mask = "?q=browse/dataitem/pixel-mask/".$request['ID'];
	
	$box = generate_isolated_pixelmask($request['ID']);
	
	if(!empty($box)):
		$request['X'] = $box['topleftx'];
		$request['Y'] = $box['toplefty'];
		$request['HEIGHT'] = $box['height'];
		$request['WIDTH'] = $box['width'];
	endif;
	
	$imagesize = getimagesize($request['PARENT_PAGE_IMAGE_PATH']);

	$js = "selectedItem = ".$request['ID'].";
		   pageimage = true;
		   tci = selectedItem;
		   loadTree();
		   var truesize = [".$imagesize[0].",".$imagesize[1]."];
		   mask = [".$request['X'].",".$request['Y'].",".$request['HEIGHT'].",".$request['WIDTH']."];
		   ";
		   
	drupal_add_js(drupal_get_path('module','dae_ui')."/javascript/jquery.Jcrop.js");
	drupal_add_js(drupal_get_path('module','dae_ui')."/javascript/jquery.Jcrop.min.js");
	drupal_add_js($js,'inline');
	
	
	$title = "Element of " . basename($request['PARENT_PAGE_IMAGE_PATH']);
	
	drupal_set_title($title);

	$rating = dae_data_get_rating($request['ID']);

	$info = array(
			'id' 		   =>  $request['ID'],
			'path' 		   =>  $image,
			'pixel_mask'   =>  $pixel_mask,
			'properties'   =>  get_pageelement_properties($request['ID']),
			'comments'     =>  get_dataitem_comments($request['ID']),
			'commentForm'  =>  get_dataitem_comment_form($request['ID']),
			'tag_form' 	   =>  get_dae_data_tag_form(),
			'toolbar'      =>  array('tag_form' => get_dae_data_tag_form()),
			'title' 	   =>  array(
								'name' 	    =>  $title, 
								'submitter' =>  get_dataitem_submitter($request['ID']), 
								'stars'  	=>  array(
												'id'     =>  $request['ID'],
												'rating' =>  $rating['rating'], 
												'rated'  =>  $rating['rated']
												)
								)
			);

	
	$ui = new PageElementPage($info);
	
	return $ui->getOutput();
}

/**
 * This function generates an alpha mask for a page image if it does not 
 * already exist and then returns the mask
 *  
 * @param [in] $dataitem - the id of the page image
 *
 * @return png binary
 */
function dae_pageimage_pixel_mask($dataitem,$datatype)
{
	require_once(drupal_get_path('module','dae_data')."/dae_data_page_elements.inc");
	
	$maskpath = "/tmp";
	
	$dataitem = check_plain($dataitem);
	$datatype = check_plain($datatype);
	
	if(empty($datatype))
		$alpha_path = $maskpath . "/" . $dataitem . "_alpha.png";
	else
		$alpha_path = $maskpath . "/" . $dataitem . "-" . $datatype ."_alpha.png";
	 
	if(!file_exists($alpha_path))
		$alpha_path = drupal_get_path('module','dae_ui')."/images/blank.png";

	$expires = 60*60*24*14;
	drupal_set_header("content-type: image/png");
	drupal_set_header("Content-Length: " . filesize($alpha_path));
	drupal_set_header("Expires: ".gmdate('D, d M Y H:i:s', time()+$expires) . ' GMT');
	drupal_set_header("Cache-Control: max-age=$expires"); ///Attempting to keep image cached
	$file = fopen($alpha_path, 'rb');
	fpassthru($file);
	fclose($file);
}

/**
 * This function outputs the html for the possible algorithms menu on data item pages when
 * /?q=browse/dataitem/runoptions/<data item id>
 *  
 * arg(3) is therefore the id of the data item 
 *
 * Echoes the html for the option list. The constructed option list URLs end up being handled
 * by the algorithms_run() callback when clicked.
 * 
 * @see algorithms_run()
 * 
 */
function dae_data_browse_runoptions()
{
	$dataitem = arg(3);
	 
	if(!check_dataset_id($dataitem)): //confirm queried dataset is just a number
		echo "Invalid Data item";
		return;
	endif;
	
	$db = getDAEDatabase();
	
	settype($dataitem, 'integer');
	$result = $db->q("select A.NAME as NAME,A.ID as ID, I.DATA_ID as INPUT
			from DATATYPE D join ASSOCIATE_DATATYPE_DATA_ITEM ASSO on D.ID = ASSO.DATATYPE_ID 
 			join TYPE_OF T on T.DATATYPE_ID = D.ID 
  			join ALGORITHM_INPUT I on I.DATA_ID = T.DATA_ID 
  			join ALGORITHM A on I.ALGORITHM_ID = A.ID 
			where DATA_ITEM_ID = " . $dataitem, array());
	
	$i = 0;
	
	echo "<span class=\"drop-down-label\">Run:</span>";
	
	while($row = $db->fetch_array($result)):
	
		/*
		 * The following function call is repsonsible for setting the local variables 
		 * 
		 * $ID (algortithm id)
		 * $INPUT (data id for input type)
		 * $NAME (algorithm name)
		 * 
		 * As per the SQL query above 
		 */
		extract($row);
	
		echo "<li id=\"algo-$ID\">
				<a rel=\"$INPUT\" href=\"?q=run/$ID/on/\">$NAME</a>
			  </li>
			  ";
		$i++;
		
	endwhile;
	
	if(!$i)
		echo "<span class=\"gray\">No suitable algorithms</span>";
	
}
/**
 * This function gets a tag form for data items
 *
 * @return - the html of for a tag form
 */
function get_dae_data_tag_form()
{
	require_once(drupal_get_path('module','dae_data')."/dae_data_tag.inc");
	return drupal_get_form('dae_data_tag_form');
}
/**
 * This function gets a comment form for data items
 *  
 * @param $dataitem - the data item the comment form will save a comment to
 *
 * @return - formatted comment form to be inserted into a page
 */
function get_dataitem_comment_form($dataitem)
{
	global $user;
	require_once(drupal_get_path("module","dae_data")."/dae_data_comment.inc");
	
	$info['form'] = drupal_get_form('dae_data_add_comment');
	$info['id'] = $dataitem;
	$info['author'] = $user->name;
	
	$info['rating'] = dae_data_get_rating($dataitem);
	
	$commentForm = new DataItemCommentForm($info);
	
	return $commentForm->getOutput();
}
function get_dataitem_submitter($did)
{
	$db = getDAEDatabase();
	settype($did, 'integer');
	$pid= $db->r("select PERSON_ID from contribute c where c.Data_Item_ID=$did",array());
	return $pid;
}

function dae_data_get_dataset_hierarchy($dataset,&$sets,&$dataset_names,$infinite_recurse_stop = array())
{
	$db = getDAEDatabase();
	settype($dataset, 'integer');
	$result = $db->q("select distinct ID,NAME from DATASET,ASSOCIATE_DATASET where ASSOCIATING_DATASET_ID=ID and DATASET_ID=$dataset ORDER BY NAME",array());
	
	while($row = $db->fetch_array($result)):
	
		$dataset_names[$row['ID']]= $row['NAME'];
	
		if(!$db->checkDataSetHasChildren($row['ID']))
			$sets[$dataset][$row['ID']] = "leaf";
		else
		{
			if(!in_array($row['ID'],$infinite_recurse_stop))
			{
				$infinite_recurse_stop[] = $row['ID'];
				dae_data_get_dataset_hierarchy($row['ID'],$sets[$dataset],$dataset_names,$infinite_recurse_stop);
			}
				
		}
	endwhile;
}
function dae_data_get_page_image_list()
{	
	$dataset = arg(3);
	if(is_numeric(arg(4)))
		$limit = arg(4);
	else
		$limit = 1;
	if(!check_dataset_id($dataset))
		return "";
		
	$ui = new PageImageList($info);
	
	dae_data_build_page_image_list($dataset, $ui,$limit);
	
	echo $ui->getOutput();
}
function dae_data_build_page_image_list($dataset, &$ui, $limit = 1)
{
	settype($dataset, 'integer');
	settype($limit, 'integer');

	$inc = 100;
	$max = $limit*$inc;
	$min = ($limit-1)*$inc;
	
	$db = getDAEDatabase();

	$no_rows = $db->r("SELECT sum(c)
					   FROM(
					   	SELECT count(PAGE_IMAGE.ID) as c from INCLUDES_PAGE_IMAGE,PAGE_IMAGE where DATASET_ID=".$dataset." and PAGE_IMAGE_ID=ID
					   	UNION
					   	SELECT count(FILES.ID) as c from INCLUDES_FILE,FILES where DATASET_ID=".$dataset." and FILE_ID=ID
					   )
					 ",array());
	
	$result  = $db->q("select ID, PATH from
					(
						SELECT ID,PATH, row_number() over(order by id) as r FROM INCLUDES_PAGE_IMAGE,PAGE_IMAGE WHERE DATASET_ID=".$dataset." and PAGE_IMAGE_ID=ID
						UNION
						SELECT ID,PATH, row_number() over(order by id) as r FROM INCLUDES_FILE,FILES WHERE DATASET_ID =".$dataset." and FILE_ID = ID
					)"
					.($limit?"where r between $min and $max":"where r > $max"),array());
	
  if($no_rows > $max)
    $ui->buildPaginNav($inc, $no_rows, $limit, $dataset);
  
	/*if($no_rows > $max && $limit)
		$ui->setLimited($dataset,$limit);*/
	
	while($row = $db->fetch_array($result)):

		$info = array('id' => $row['ID'], 'path' => $row['PATH'], 'thumb' => dae_data_get_thumb_uri($row['ID'], 'medium'));
		$ui->addPageImage($info);
	
	endwhile;
	
	if(!$no_rows):
		$info['id'] = 0;
		$info['path'] = "Empty";
		$ui->addPageImage($info);
		return false;
	endif;

	return true;
}
/**
 * This function returns information on a dataset
 *  
 * @param $dataitem - data item id
 *
 * @return - an array of information on the dataset
 */
function dae_data_get_dataset_info_array($dataset)
{
    $db = getDAEDatabase();
    
	settype($dataset, 'integer');
	
	$result = $db->q("select NAME,ID from DATASET where ID = ". $dataset, array());
	
	$dataset = $db->fetch_array($result);
	
	return $dataset;

}
/**
 * This function returns information on a page image
 *  
 * @param $dataitem - data item id
 *
 * @return - an array of information on the page image
 */
function dae_data_get_pageimage_info_array($dataset)
{
	$db = getDAEDatabase();
	
	settype($dataset, 'integer');
	
	$result = $db->q("select ID,PATH from PAGE_IMAGE where ID = ". $dataset, array());
	
	$data = $db->fetch_array($result);
	
	return $data;

}

/**
 * This function returns information on a file
 *  
 * @param $dataitem - data item id
 *
 * @return - an array of information on the page image
 */
function dae_data_get_file_info_array($dataset)
{
	$db = getDAEDatabase();
	
	settype($dataset, 'integer');
	
	$result = $db->q("select ID,PATH from FILES where ID = ". $dataset, array());
	
	$data = $db->fetch_array($result);
	
	return $data;

}

/**
 * This function returns information on a page element
 *  
 * @param $dataitem - data item id
 *
 * @return - an array of information on the page element
 */
function dae_data_get_page_element_info_array($dataitem)
{
	$db = getDAEDatabase();
		
	settype($dataitem, 'integer');
	
	$result = $db->q("SELECT PAGE_IMAGE.ID as PARENT_PAGE_IMAGE_ID, PATH as PARENT_PAGE_IMAGE_PATH, PAGE_ELEMENT.WIDTH, PAGE_ELEMENT.HEIGHT, TOPLEFTX AS X, TOPLEFTY AS Y
					  FROM PAGE_IMAGE JOIN CONTAINS_PAGE_ELEMENT ON PAGE_IMAGE.ID = PAGE_IMAGE_ID JOIN PAGE_ELEMENT ON PAGE_ELEMENT.ID = PAGE_ELEMENT_ID 
					  WHERE PAGE_ELEMENT.ID = $dataitem", array());
	
	$data = $db->fetch_array($result);
	
	$data['ID'] = $dataitem;
	
	return $data;
}
/**
 * This function saves a users rating of a data item to the database
 *  
 * @param arg(3) the data item id
 * @param arg(4) the rating (1-5)
 *
 */
function dae_data_browse_dataitem_rate()
{
	$rating = arg(4);
	$dataitem = arg(3);
	$db = getDAEDatabase();
	
	global $user;
	
	if(!check_dataset_id($dataitem))
		return "";
	
	if(!is_numeric($rating) || $rating > 5 || $rating < 1)
		return "Invalid Rating";
		
	settype($dataitem, 'integer');
		
	if(dae_data_rating_exists($dataitem)):
		$db->q("UPDATE DAE_DATAITEM_RATING SET RATING = $rating WHERE USER_ID = ".getPID($user->uid)." AND DATA_ITEM = $dataitem",array());
	else:
		$db->q("INSERT INTO DAE_DATAITEM_RATING VALUES($dataitem,".getPID($user->uid).",$rating)",array());
	endif;

}
/**
 * This function saves a helpful rating to the database
 *
 * @return - Success if a comment rating is saved
 */
function dae_data_browse_dataset_comment_rate()
{
	$helpful = arg(4);
	$comment = arg(3);
	
	global $user;
	
	if(!check_comment_id($comment))
		return "";
	
	if(!is_numeric($helpful) || $helpful > 1 || $helpful < 0):
		
		drupal_add_js(drupal_get_path("module","dae_ui") . '/javascript/dae_data_pages.js');
		
		$db = getDAEDatabase();
		
		$comment_information = get_comment_information($comment);
					
		$ui = new DataItemComment();
	
		$ui->addComment($comment_information);
		
		return $ui->getOutput();
		
	else:
		if(dae_data_comment_rating_exists($comment)):
			db_query("update {dae_dataitem_comment_helpful} set helpful = ".$helpful." where rater = ".$user->uid." and cid = ". $comment);
		else:
			db_query("insert into {dae_dataitem_comment_helpful} values($comment,".$user->uid.",$helpful)");
		endif;
		
		echo "Success";
	endif;
}
/**
 * This builds the html for the comments on a data item
 *  
 * @param $dataitem - the id of the data item to return comments for
 *
 * @return - HTML string with the comments
 */
function get_dataitem_comments($dataitem)
{
	$db = getDAEDatabase();
	$ui = new DataItemComment();
		
	settype($dataitem, 'integer');
	
	$comments  = $db->q("SELECT ID, BODY, COMMENT_DATE, COMMENTED_BY, SUM(HELPFUL) AS YES, COUNT(HELPFUL) AS TOTAL 
						FROM DAE_DATAITEM_COMMENT 
						LEFT OUTER JOIN DAE_DATAITEM_COMMENT_HELPFUL ON ID = CID WHERE C_ON = $dataitem 
						GROUP BY ID, BODY, COMMENT_DATE, COMMENTED_BY 
						ORDER BY SUM(HELPFUL)/COUNT(HELPFUL) DESC, COUNT(HELPFUL) DESC",array());
	
	
	while($row = $db->fetch_array($comments)):
		$info = array(
			'id' => $row['id'],
			'author_id' => $row['by'], 
			// changed from author to author_id, for complete person info
			// old verson: 
			// 'author' => $db->r("select NAME from PERSON where id = ". $row['by']),
			'stars' => array('rating' => db_result(db_query("select rating from {dae_dataitem_rating} where uid = ".$row['by']." and id = $dataitem"))/5),
			'helpful' => array( 'yes' => $row['yes'], 'total' => $row['total']),
			'body' => $row['body'],
			'date' =>$row['date'],
			'limit' => true,
		);
	
	$ui->addComment($info);
	
	endwhile;
	
	return $ui->getOutput();
}
/**
 * This function gets an array of information on a comment
 *  
 * @param $comment - the comment id
 *
 * @return - an array containing the id, author, rating information, text and date
 */
function get_comment_information($comment)
{
	$db = getDAEDatabase();
	
	$comments  = db_query("select *,sum(helpful) as yes,count(helpful) as total 
						from {dae_dataitem_comment} left outer join {dae_dataitem_comment_helpful} on id = cid 
						where id = ".$comment." 
					");
	
	$row = db_fetch_array($comments);
		
	$info = array(
		'id' => $row['id'],
		'author' => $db->r("select NAME from PERSON where id = ". $row['by'],array()),
		'stars' => array('rating' => db_result(db_query("select rating from {dae_dataitem_rating} where uid = ".$row['by']." and id = ".$row['c_on']))/5),
		'helpful' => array( 'yes' => $row['yes'], 'total' => $row['total']),
		'body' => $row['body'],
		'date' =>$row['date']
	);
	
	return $info;
}
/**
 * This function confirms a comment id exists
 *  
 * @param $comment - the comment id
 *
 * @return - 0 if no comment exists, otherwise positive integer
 */
function check_comment_id($comment)
{
	$db = getDAEDatabase();
		
	settype($comment, 'integer');
	
	return $db->q("SELECT COUNT(ID) FROM DAE_DATAITEM_COMMENT WHERE ID = $comment",array());
}
/**
 * This function checks to see if a comment has already been rated
 *  
 * @param $comment - the comment id
 *
 * @return - 0 if no rating exists, otherwise a positive integer
 */
function dae_data_comment_rating_exists($comment)
{
	global $user;
	$db = getDAEDatabase();
		
	settype($comment, 'integer');
	
	return $db->q("SELECT COUNT(*) FROM DAE_DATAITEM_COMMENT_HELPFUL WHERE CID = $comment AND RATER = ".getPID($user->uid), array());
}
/**
 * This function declares the form for creating page elements
 *
 * @return - a drupal form array
 */
function dae_data_page_element_form()
{
	global $db;
	require_once(drupal_get_path('module','dae_data')."/dae_data_page_elements.inc");		

	//$form = dae_page_element_add_interpretation(array(),arg(2));
	$dataitem = arg(2);

	$form['coordsbar'] = array('#type' => 'markup', '#value' => '<div id="page-element-coords-bar">', '#weight' => -3);

	$form['dae_data_page_element_x1'] = array(
	    '#type' => 'textfield', 
	  	'#size' => 5,
	  	'#title' => 'X1',
	  	'#default_value' => 0,
	  	'#weight' => -2,
	);
	$form['dae_data_page_element_y1'] = array(
		'#type' => 'textfield', 
	  	'#size' => 5,
	  	'#title' => 'Y1',
	  	'#default_value' => 0,
	  	'#weight' => -2,
	);
	$form['dae_data_page_element_x2'] = array(
		'#type' => 'textfield', 
	  	'#size' => 5,
	  	'#title' => 'X2',
	  	'#default_value' => 0,
	  	'#weight' => -2,
	);
	$form['dae_data_page_element_y2'] = array(
		'#type' => 'textfield', 
	  	'#size' => 5,
	  	'#title' => 'Y2',
	  	'#default_value' => 0,
	  	'#weight' => -2,
	);
	$form['dae_data_page_element_submit'] = array(
		'#type' => 'button',
		'#value' => 'Save',
		'#weight' => -2,
	);
	$form['dae_data_page_element_page_image'] = array(
		'#type' => 'hidden',
		'#required' => true,
		'#weight' => -2,
	);
	
	$form['infoform'] = array('#type' => 'markup', '#value' => '</div><div id="page-element-info-form">', '#weight' => -1);
	
	
		$types   = 	  $db->q("SELECT distinct NAME, D.ID
						  FROM DATATYPE D JOIN ASSOCIATE_DATATYPE_DATA_ITEM ADDI ON ADDI.DATATYPE_ID = D.ID
						  WHERE ADDI.DATA_ITEM_ID IN(
						  			SELECT CPE.PAGE_ELEMENT_ID
						  			FROM INCLUDES_PAGE_IMAGE IPI 
						  			JOIN CONTAINS_PAGE_ELEMENT CPE ON IPI.PAGE_IMAGE_ID = CPE.PAGE_IMAGE_ID
						  			WHERE DATASET_ID IN(
				                      SELECT DATASET_ID
				                      FROM INCLUDES_PAGE_IMAGE IPI 
				                      JOIN CONTAINS_PAGE_ELEMENT CPE ON IPI.PAGE_IMAGE_ID = CPE.PAGE_IMAGE_ID
				                      WHERE CPE.PAGE_ELEMENT_ID = $dataitem OR IPI.PAGE_IMAGE_ID = $dataitem

						  		)
						  ) ORDER BY NAME",array());
						  
	$options = array();

	while($type = $db->fetch_array($types)):
		if(strlen($type['NAME']))
			$options[$type['ID']] = $type['NAME'];
	endwhile;
	
	$options['null'] = "--Other Options--";

	$types   = 	  $db->q("
					SELECT distinct NAME, ID
					FROM DATATYPE
					WHERE ID NOT IN (
	
						SELECT D.ID
					  FROM DATATYPE D JOIN ASSOCIATE_DATATYPE_DATA_ITEM ADDI ON ADDI.DATATYPE_ID = D.ID
					  WHERE ADDI.DATA_ITEM_ID IN(
					  			SELECT CPE.PAGE_ELEMENT_ID
					  			FROM INCLUDES_PAGE_IMAGE IPI 
					  			JOIN CONTAINS_PAGE_ELEMENT CPE ON IPI.PAGE_IMAGE_ID = CPE.PAGE_IMAGE_ID
					  			WHERE DATASET_ID IN(
			                      SELECT DATASET_ID
			                      FROM INCLUDES_PAGE_IMAGE IPI 
			                      JOIN CONTAINS_PAGE_ELEMENT CPE ON IPI.PAGE_IMAGE_ID = CPE.PAGE_IMAGE_ID
			                      WHERE CPE.PAGE_ELEMENT_ID = $dataitem OR IPI.PAGE_IMAGE_ID = $dataitem
					)
				  	)
				  ) AND (DATA_ITEM_TYPE is NULL OR DATA_ITEM_TYPE = 'page_element') ORDER BY NAME",array());

	$exists = array();

	while($type = $db->fetch_array($types)):
		if(strlen($type['NAME']) && !in_array($type['NAME'],$exists)){
			$exists[] = $type['NAME'];
			$options[$type['ID']] = $type['NAME'];
		}
	endwhile;

	if(count($options)){

		$form['howPEType'] = array(
			'#type' => 'radios',
			'#default_value' => 1,
			'#title' => t('Page Element Type'),
	    	'#options' => array(1 => 'Use Existing', 2 => 'Create New'),
	    );
	
		$form['PEtypeA'] = array(
			'#type' => 'select',
	       	'#options' => $options,
		);
		$form['PEtypeB'] = array(
			'#type' => 'textfield',
		);

	}
	else{
		$form['PEtypeB'] = array(
			'#type' => 'textfield',
			'#title' => t('Interpretation Type'),
		);
	}
/*
	$form['PEtypeBMatchingID'] = array(
		'#type' => 'hidden',
	);
	*/
	$form['submit'] = array('#type' => 'submit', '#value' => t('Add'));

	
	$form['closeform'] = array('#type' => 'markup', '#value' => '</div>', '#weight' => 1);
	
	return $form;
}
function dae_data_page_element_form_validate()
{
	// @todo add validation for this form
	
	if(!user_access('add element'))
		form_set_error('','You are not authorized to add page elements.');
	return;
}
/**
 * This function saves a user created page element into the database
 *  
 * @param $form_id - the druapl form id
 * @param $form_values - the user submitted form values
 *
 * @return - none
 */
function dae_data_page_element_form_submit($form_id, $form_values)
{
	global $user;
	$db = getDAEDatabase();
	
	if(user_access('create element'))
	{
		$db->i("insert into DATA_ITEM (ID, STATUS, DESCRIPTION, COPYRIGHT) values (seq_data_item.nextval, 0, 'Web created page element property value', '')",array());
		$dataitem = $db->r("SELECT seq_data_item.CURRVAL FROM DUAL",array());
		$db->i("insert into IMAGE_DATA_ITEM (ID) values ($dataitem)",array());
		$db->i("insert into PHYSICAL_IMAGE_DATA_ITEM (id) values ($dataitem)",array());
		$db->i("insert into CONTRIBUTE values(".getPID($user->uid).",$dataitem, null, 'Web created page element')",array());
				
		settype($form_values['values']['dae_data_page_element_x1'], 'integer');
		settype($form_values['values']['dae_data_page_element_x2'], 'integer');
		settype($form_values['values']['dae_data_page_element_y1'], 'integer');
		settype($form_values['values']['dae_data_page_element_y2'], 'integer');
		settype($form_values['values']['dae_data_page_element_page_image'], 'integer');
			
		$db->i("INSERT INTO PAGE_ELEMENT (ID, WIDTH, HEIGHT, TOPLEFTX, TOPLEFTY) 
				VALUES($dataitem,".
				($form_values['values']['dae_data_page_element_x2']-$form_values['values']['dae_data_page_element_x1']).",".
				($form_values['values']['dae_data_page_element_y2']-$form_values['values']['dae_data_page_element_y1']).",".
				 $form_values['values']['dae_data_page_element_x1'].",".
				 $form_values['values']['dae_data_page_element_y1'].")",array());
																						 
		$db->i("INSERT INTO CONTAINS_PAGE_ELEMENT VALUES($dataitem,".$form_values['values']['dae_data_page_element_page_image'].")",array());
	
		/**
		 * Create datatype for page element if necessary.  Associate page element and datatype
		 *
		 */
		
		$datatype = $form_values['values']['PEtypeA'];

		if($form_values['values']['howPEType'] != 1){
					
			$name = check_plain($form_values['values']['PEtypeB']);
			if(empty($name)){
				form_set_error('','You must specify a datatype');
				return;
			}
			$db->i("INSERT INTO DATATYPE (ID,NAME,DESCRIPTION,DATA_ITEM_TYPE) values(SEQ_DATATYPE.NEXTVAL,'$name','Web added page element datatype','page_element')",array());
			$datatype = $db->r("SELECT SEQ_DATATYPE.CURRVAL FROM DUAL",array());	
		}
		
		$db->i("INSERT INTO ASSOCIATE_DATATYPE_DATA_ITEM VALUES($dataitem,$datatype)",array());
	
		/**
		 * Create page element property value datatype if necessary.  Create property value
		 * and associate with datatype and page element
		 */
		if(!empty($form_values['values']['value'])){
	
			settype($datatype,'integer');
			$datatype = $form_values['values']['typeA'];
			
			//create datatype if necessary
			if(!isset($form_values['values']['how']) || $form_values['values']['how'] == 2){
			
					
				$name = check_plain($form_values['values']['typeB']);
				if(empty($name)){
					form_set_error('','You must specify a datatype');
					return;
				}
				
				settype($existing,'integer');
				$existing = $form_values['values']['PEtypeBMatchingID'];
				
				if($db->r("SELECT count(*) FROM DATATYPE WHERE ID = $existing && NAME = '$name'",array())){
					$datatype = $existing;
				}
				else{
					$db->i("INSERT INTO DATATYPE_PROPERTY (ID,NAME,DESCRIPTION) values(SEQ_PROPERTY.NEXTVAL,'$name','Web added page element property value datatype')",array());
					$datatype = $db->r("SELECT SEQ_PROPERTY.CURRVAL FROM DUAL",array());	
				}
			}
			settype($page_element,'integer');
			$page_element = $dataitem;
			$interpretation = check_plain($form_values['values']['value']);
			
			$db->i("insert into DATA_ITEM (ID, STATUS, DESCRIPTION, COPYRIGHT) values (seq_data_item.nextval, 0, 'Web created page element property value', '')",array());
			$pepv = $db->r("SELECT seq_data_item.CURRVAL FROM DUAL",array());
			$db->i("insert into IMAGE_DATA_ITEM (ID) values ($pepv)",array());
			$db->i("insert into LOGICAL_IMAGE_DATA_ITEM (id) values ($pepv)",array());
			$db->i("INSERT INTO PAGE_ELEMENT_PROPERTY_VALUE (ID,VALUE_TYPE,VALUE) VALUES ($pepv,null,'$interpretation')",array());
			$db->i("INSERT INTO HAS_VALUE VALUES($pepv,$page_element)",array());
			$db->i("INSERT INTO VALUE_OF VALUES($pepv,$datatype)",array());
			$db->i("insert into CONTRIBUTE values(".getPID($user->uid).",$pepv, null, 'Web created page element property value')",array());
		
			drupal_goto('browse/dataitem/'.$page_element);
			return;
		}
		drupal_goto('browse/dataitem/'.$form_values['values']['dae_data_page_element_page_image'],null,'show-elements');
		return;
	}
	else
	{
		drupal_set_message(t("You are not authorized to create page elements"),'warning');
	}
	
}
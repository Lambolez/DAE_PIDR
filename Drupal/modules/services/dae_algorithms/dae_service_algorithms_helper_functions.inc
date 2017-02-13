<?php

/**
 * @file dae_service_algorithms_helper_functions.inc
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
 *
 */
 
 
/**
 *  ADDING ALGORITHM HOOKS:
 *
 *  There are two flavors of hooks, those based on ID, and those based on name.
 *  By their nature, hooks by ID will only be called for the exact algorithm,
 *	while hooks by name can refer multiple versions of an algorithm assuming
 *  all of them have the same name in the database.  
 *
 *  Hooks by name may be more flexible accross databases where IDs are arbitrary.
 *
 *  Currently, BOTH hooks are called, so if you hook convert and 102 (convert's ID)
 *  both will be run.
 *
 *  ARGUMENTS:
 *
 *	Hooks get one argument which is an array of information.
 *	\todo add what is in the array as necessary
 *
 *
 *  FORMAT:
 *	
 *  By name: hook_<name>_<version> "hook_kanungo_degradation_1_0($arguments)" where name is entirely lowercase 
 * 			 and all non-alphanumeric have been replaced with underscores.
 *
 *  By id:   hook_<id> "hook_124($arguments)"
 *
 */
 
require_once('./sites/default/misc/identify.php');

function create_thumbnails($file){

	$script =  "/opt/csw/bin/convert -resize 75x75 '$file' '$file-tn-75.png'; ".
			   "/opt/csw/bin/convert -resize 250x200 '$file' '$file-tn-250.png'; ".
			   "/opt/csw/bin/convert -resize '1000x1000>' '$file' '$file-tn-1000.png'; ";
			   
	return (system($script) == 0);

}

/**
 *  This function hooks the ocrad algorithm (v 0.19)
 *
 *
 *
 */
 
function hook_ocrad_0_21_2($args = array()){
	generic_hook_ocrad($args);
}
	
function hook_ocrad_0_19($args = array()){
	generic_hook_ocrad($args);
}

function generic_hook_ocrad($args = array()){//still need to pass args
		
	$db = getDAEDatabase();	
	global $user;
	
	$run_vars[':id'] = $args['id'];
	
	$getitems = $db->q("SELECT F.PATH AS OUTPUT, DI2.ID AS INPUT, PI.PATH AS IMAGE, PI.WIDTH AS WIDTH, PI.HEIGHT as HEIGHT 
						FROM ALGORITHM_RUN_OUTPUT ARO JOIN DATA_ITEM DI ON DATA_ITEM_ID = DI.ID JOIN FILES F ON F.ID = DI.ID 
						JOIN ALGORITHM_RUN_INPUT ARI ON ARI.ALGORITHM_RUN_ID = ARO.ALGORITHM_RUN_ID 
						JOIN DATA_ITEM DI2 ON ARI.DATA_ITEM_ID = DI2.ID JOIN PAGE_IMAGE PI ON PI.ID = DI2.ID
						WHERE ARO.ALGORITHM_RUN_ID = :id AND NAME LIKE '%.txt'",$run_vars);
		 
	$dataitems = $db->fetch_array($getitems);
	
	$db->i("insert into DATA_ITEM (ID, STATUS, DESCRIPTION, COPYRIGHT, FLAG) values (seq_data_item.nextval, 0, 'Ocrad Web Service PE', '', 'page_element')",array());
	$dataitem = $db->r("SELECT seq_data_item.CURRVAL FROM DUAL",array());
	$db->i("insert into IMAGE_DATA_ITEM (ID) values ($dataitem)",array());
	$db->i("insert into PHYSICAL_IMAGE_DATA_ITEM (id) values ($dataitem)",array());
	$db->i("insert into CONTRIBUTE values(".getPID($user->uid).",$dataitem, null, 'Web created page element')",array());
		
	$width = $dataitems['WIDTH'];
	$height = $dataitems['HEIGHT'];
	
	if(!$width || !$height)
		list($width, $height, $type, $attr) = linpha_getimagesize($dataitems['IMAGE']);
	
	error_log('generic_hook_ocrad (' . $width . ',' . $height . ')');
	
	/*
	 * Sometimes getimagesize fails on uncommon or too large images 
	 * In that case, we don't register width and heigth.
	 */
	$vars[':contents'] = file_get_contents($dataitems['OUTPUT']);

	if(!$width || !$height) {
	    $db->i("INSERT INTO PAGE_ELEMENT (ID, WIDTH, HEIGHT, TOPLEFTX, TOPLEFTY) VALUES($dataitem,0,0,0,0)",array());
	}
	else {
	    $db->i("INSERT INTO PAGE_ELEMENT (ID, WIDTH, HEIGHT, TOPLEFTX, TOPLEFTY) VALUES($dataitem,".$width.",".$height.",0,0)",array());
	}
																 
	$db->i("INSERT INTO CONTAINS_PAGE_ELEMENT VALUES($dataitem,".$dataitems['INPUT'].")",array());
	
	$db->i("insert into DATA_ITEM (ID, STATUS, DESCRIPTION, COPYRIGHT, FLAG) values (seq_data_item.nextval, 0, 'Ocrad Web Service PEPV', '', 'page_element_property_value')",array());
	$value = $db->r("SELECT seq_data_item.CURRVAL FROM DUAL",array());
	$db->i("insert into IMAGE_DATA_ITEM (ID) values ($value)",array());
	$db->i("insert into LOGICAL_IMAGE_DATA_ITEM (id) values ($value)",array());
	
	$db->i("INSERT INTO PAGE_ELEMENT_PROPERTY_VALUE (ID, VALUE) VALUES ($value,:contents)",$vars);
	
	$db->i("INSERT INTO HAS_VALUE (PAGE_ELEMENT_PROPERTY_VALUE_ID, PAGE_ELEMENT_ID) VALUES ($value, $dataitem)",array());
}
 
/**
 *  This function hooks the convert algorithm (v 6.4.6)
 *
 *
 *
 */
function hook_convert_6_6_0_4($args = array()){
	generic_hook_convert($args);
}

function hook_convert_6_4_6($args = array()){
	generic_hook_convert($args);
}

function generic_hook_convert($args = array()){

	
 	$db = getDAEDatabase();
 	
 	$run_vars[':id'] = $args['id'];
 	
 	$output = $db->q("SELECT ID, PATH
					 FROM ALGORITHM_RUN_OUTPUT ARO JOIN PAGE_IMAGE PI ON DATA_ITEM_ID = PI.ID 
					 WHERE ARO.ALGORITHM_RUN_ID = :id",$run_vars);
					 
	$file = $db->fetch_array($output);
 
	if(!$file['PATH'] || !create_thumbnails($file['PATH']))
		watchdog('Error',t('Thumbnail for %dataitem created by run %run could not be created.',array('%dataitem' => $file['ID'], '%run' => $run_vars[':id'])));
 
}

<?php
/**
 * @file dae_data_page_elements.inc
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


/**
 * This function returns an array of page element property values for the given element
 *  
 * @param [in] $dataitem - the page element to find values for
 *
 * @return an array containing all property values and types
 */
function get_pageelement_properties($dataitem)
{
	global $db;
    settype($dataitem, 'integer');
	$properties = $db->q("SELECT D.NAME, VALUE
						 FROM PAGE_ELEMENT_PROPERTY_VALUE
						 JOIN HAS_VALUE ON PAGE_ELEMENT_PROPERTY_VALUE.ID = 
						 has_value.PAGE_ELEMENT_PROPERTY_VALUE_ID
						 LEFT JOIN Value_of ADDI ON addi.PAGE_ELEMENT_PROPERTY_VALUE_ID = 
						 has_value.PAGE_ELEMENT_PROPERTY_VALUE_ID
						 LEFT JOIN DATATYPE_property D ON ADDI.DATA_TYPE_Property_ID = D.ID
						 WHERE PAGE_ELEMENT_ID = $dataitem",array());
	
	
	$values = array();
	
	while($property = $db->fetch_array($properties)):
		
		$values[] = array('type' => $property['NAME'],'value' => "<pre>".check_plain((is_null($property['VALUE'])) ? '' : $property['VALUE']->load())."</pre>");
	
	endwhile;
	
	$values[] = array('type' => 'Add Interpretation', 'value' => drupal_get_form('dae_page_element_add_interpretation',$dataitem));
	
	return $values;

}

/**
 * This function returns a drupal form array for creating interpretations of page elements
 *  
 * @param [in] $dataitem - the page element for which an interpretation should be added
 *
 * @return an array containing a drupal form
 */
function dae_page_element_add_interpretation($form_state,$dataitem){

	global $db;
	
	$types   = 	  $db->q("SELECT distinct NAME, D.ID
						  FROM DATATYPE_PROPERTY D JOIN VALUE_OF ADDI ON ADDI.DATA_TYPE_PROPERTY_ID = D.ID
						  WHERE ADDI.PAGE_ELEMENT_PROPERTY_VALUE_ID IN(
						  		SELECT PAGE_ELEMENT_PROPERTY_VALUE_ID
						  		FROM HAS_VALUE 
						  		WHERE PAGE_ELEMENT_ID IN (
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
						  )",array());
						  
	$options = array();

	while($type = $db->fetch_array($types)):
		if(strlen($type['NAME']))
			$options[$type['ID']] = $type['NAME'];
	endwhile;

	$options['null'] = "--Other Options--";

	$types   = 	  $db->q("
					SELECT distinct NAME, ID
					FROM DATATYPE_PROPERTY
					WHERE ID NOT IN (
	
					SELECT D.ID
						  FROM DATATYPE_PROPERTY D JOIN VALUE_OF ADDI ON ADDI.DATA_TYPE_PROPERTY_ID = D.ID
						  WHERE ADDI.PAGE_ELEMENT_PROPERTY_VALUE_ID IN(
						  		SELECT PAGE_ELEMENT_PROPERTY_VALUE_ID
						  		FROM HAS_VALUE 
						  		WHERE PAGE_ELEMENT_ID IN (
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
						  )
				  ) ORDER BY NAME",array());

	$exists = array();

	while($type = $db->fetch_array($types)):
		if(strlen($type['NAME']) && !in_array($type['NAME'],$exists)){
			$exists[] = $type['NAME'];
			$options[$type['ID']] = $type['NAME'];
		}
	endwhile;


	if(count($options)){

		$form['how'] = array(
			'#type' => 'radios',
			'#default_value' => (empty($options)?2:1),
			'#title' => t('Interpretation Type'),
	    	'#options' => array(1 => 'Use Existing', 2 => 'Create New'),
	    	'#required' => true
	    );
	
		$form['typeA'] = array(
			'#type' => 'select',
	       	'#options' => $options,
		);
		$form['typeB'] = array(
			'#type' => 'textfield',
		);

	}
	else{
		$form['typeB'] = array(
			'#type' => 'textfield',
			'#title' => t('Interpretation Type'),
		);
		
	
	}
	$form['page_element'] = array(
		'#value' => $dataitem,
		'#type' => 'hidden'
	);
	
	$form['value'] = array(
    	'#type' => 'textarea',
    	'#title' => t('Interpretation'),
    	'#cols' => 60,
    	'#rows' => 5,
    	'#required' => true,
    	'#description' => t('Add your own interpretation of this page element'),
  	);

	$form['submit'] = array('#type' => 'submit', '#value' => t('Add'));
	return $form;

}
/**
 * This function validates the input from the user for the interpretation form
 *  
 * @param [in] $form - standard drupal form validate 
 * @param [in] $form_state - standard for drupal form validate
 *
 *
 */
function dae_page_element_add_interpretation_validate($form, &$form_state){

	if(isset($form_state['values']['how'])){
	
		if($form_state['values']['how'] == 2){
		
			if(!strlen($form_state['values']['typeB']))
				form_set_error('', t('You must provide a name for your type.'));
		}
	
	}
	if(!user_access('add property value'))
		form_set_error('','You are not authorized to add page element property values.');


}
/**
 * This function saves the input from the user for the interpretation form
 *  
 * @param [in] $form - standard drupal form submit
 * @param [in] $form_state - standard for drupal form submit
 *
 *
 */
function dae_page_element_add_interpretation_submit($form, &$form_state){

	global $db, $user;

	settype($datatype,'integer');
	$datatype = $form_state['values']['typeA'];
	
	//create datatype if necessary
	if(!isset($form_state['values']['how']) || $form_state['values']['how'] == 2){
		
		$name = check_plain($form_state['values']['typeB']);
		if(empty($name)){
			form_set_error('','You must specify a datatype');
			return;
		}
		$db->i("INSERT INTO DATATYPE_PROPERTY (ID,NAME,DESCRIPTION) values(SEQ_PROPERTY.NEXTVAL,'$name','Web added page element property value datatype')",array());
		$datatype = $db->r("SELECT SEQ_PROPERTY.CURRVAL FROM DUAL",array());	
	}
	settype($page_element,'integer');
	$page_element = $form_state['values']['page_element'];
	$interpretation = check_plain($form_state['values']['value']);
	
	$db->i("insert into DATA_ITEM (ID, STATUS, DESCRIPTION, COPYRIGHT) values (seq_data_item.nextval, 0, 'Web created page element property value', '')",array());
	$pepv = $db->r("SELECT seq_data_item.CURRVAL FROM DUAL",array());
	$db->i("insert into IMAGE_DATA_ITEM (ID) values ($pepv)",array());
	$db->i("insert into LOGICAL_IMAGE_DATA_ITEM (id) values ($pepv)",array());
	$db->i("INSERT INTO PAGE_ELEMENT_PROPERTY_VALUE (ID,VALUE_TYPE,VALUE) VALUES ($pepv,null,'$interpretation')",array());
	$db->i("INSERT INTO HAS_VALUE VALUES($pepv,$page_element)",array());
	$db->i("INSERT INTO VALUE_OF VALUES($pepv,$datatype)",array());
	$db->i("insert into CONTRIBUTE values(".getPID($user->uid).",$pepv, null, 'Web created page element property value')",array());


	drupal_set_message("Page element property value added.","status");

}

/**
 * Generates a transparent pixelmask for the \a page_element of which its \a id is given.
 * 
 * @param [in] $elementid the id of the \a page_element
 * 
 * \todo This code is currently only taking into account the very specifi syntax for \a CVC-cc-area
 * type representations. This should be modified to a more generic framework using the \a boundary
 * attribute of the \a page_element.
 */
function generate_isolated_pixelmask($elementid,$path = '/tmp')
{
	global $db;
    settype($elementid, 'integer');
	$query = "SELECT MASK FROM PAGE_ELEMENT WHERE ID = $elementid AND MASK IS NOT NULL";
	//select mask if it exists
	
	$exists = $db->r("select count(*) from ($query)",array());
	
	if($exists < 1)
		return false;

	$coordinates = $db->q($query,array());
	
	$coordinates = $db->fetch_array($coordinates);
	
	$coords = convert_db_mask_to_array($coordinates['MASK']);
		
	$corners = getMinMaxCoords($coords);
	extract($corners);
	//get x y min and maxes
	
	$width = $xmax - $xmin;
	$height = $ymax - $ymin;

	$box = array('topleftx'  => $xmin,
				 'toplefty'  => $ymin,
				 'width'     => $width,
				 'height'    => $height,
		   );


	$pixels = array_fill(0,$height*$width,0);
	
	foreach ($coords['x'] as $index => $x):
			
		$pixels[($coords['y'][$index]-$ymin)*$width+($x-$xmin)] = 255;
		//map to image
	
	endforeach;
	
	makePixelMask($elementid,$path,$width,$height,$pixels);
	
	return $box;
}


/**
 * Generates a transparent pixelmask for the \a page_element of which its \a id is given.
 * 
 * @param [in] $elementid the id of the \a page_element
 * 
 * \todo This code is currently only taking into account the very specifi syntax for \a CVC-cc-area
 * type representations. This should be modified to a more generic framework using the \a boundary
 * attribute of the \a page_element.
 */
function generate_pixelmask($pageimage,$path = "/tmp")
{
	global $db;
	
    settype($pageimage, 'integer');
	$imagesize = getimagesize($db->r("SELECT PATH FROM PAGE_IMAGE WHERE ID = $pageimage",array()));
	$width = $imagesize[0];
	$height = $imagesize[1];
	//get the size of the actual page image for mask size
	
	$query = "SELECT MASK, PE.ID, DT.NAME, DT.ID AS DT, TOPLEFTX, TOPLEFTY, WIDTH, HEIGHT
			FROM PAGE_ELEMENT PE, HAS_VALUE, PAGE_ELEMENT_PROPERTY_VALUE PEPV, 
				 DATATYPE_PROPERTY DP, VALUE_OF V, CONTAINS_PAGE_ELEMENT CPE, 
				 ASSOCIATE_DATATYPE_DATA_ITEM ADDT, DATATYPE DT
			WHERE DP.ID = V.DATA_TYPE_PROPERTY_ID
			and ADDT.DATA_ITEM_ID = PE.ID
			and ADDT.DATATYPE_ID = DT.ID
			and CPE.PAGE_ELEMENT_ID = PE.ID
			and V.PAGE_ELEMENT_PROPERTY_VALUE_ID = PEPV.ID 
			and PE.ID = HAS_VALUE.PAGE_ELEMENT_ID 
			and HAS_VALUE.PAGE_ELEMENT_PROPERTY_VALUE_ID = PEPV.ID
			and MASK IS NOT NULL
			and CPE.PAGE_IMAGE_ID = $pageimage";
	
	$nbMasks = $db->r("select count(*) from ($query)",array());
	
	if($nbMasks == 0)
		return array();
	
	$coordQuery = $db->q($query,array());	
	
	$coordinates = $db->fetch_array($coordQuery);
	
	$pixels = array();
	
	$existingMasks = array();
	
	do{
		
		//if(!empty($coordinates['TOPLEFTX']) && !empty($coordinates['TOPLEFTY']) && !empty($coordinates['WIDTH']) && !empty($coordinates['HEIGHT'])){
			
			$boxes[$coordinates['NAME']][$coordinates['ID']] = array(
					 'left'   => $coordinates['TOPLEFTX'],
					 'top'    => $coordinates['TOPLEFTY'],
					 'width'  => $coordinates['WIDTH'],
					 'height' => $coordinates['HEIGHT'],
					 'name'   => "Mask " . (count($boxes[$coordinates['NAME']])+1),
					 'mask'   => $coordinates['DT']
			   );
			 //prevent mask mistake
			 if(file_exists($path . "/" . $coordinates['ID'] . "-" . $coordinates['DT'] . "_alpha.png")):
			 	$existingMasks[] = $coordinates['DT'];
			endif;
		//}
		//else{
			//if(in_array($coordinates['DT'],$existingMasks))
			//	continue;
			
			$coords = convert_db_mask_to_array($coordinates['MASK']);
			
			$element_data_type = $coordinates['DT'];
			
			foreach ($coords['x'] as $index => $x):
				
				$pixels[$element_data_type][($coords['y'][$index])*$width+$x] = 255;
				//$aggregateMask[($coords['y'][$index])*$width+$x] = 255;
				
			endforeach;
			//map xy pairs to an array of all pixels for a given datatype in the image
			
			$corners = getMinMaxCoords($coords);
			extract($corners);
			
			$db->q("UPDATE PAGE_ELEMENT SET WIDTH = ".($xmax - $xmin)." WHERE ID = ".$coordinates['ID']." AND WIDTH IS NULL",array());
			$db->q("UPDATE PAGE_ELEMENT SET HEIGHT = ".($ymax - $ymin)." WHERE ID = ".$coordinates['ID']." AND HEIGHT IS NULL",array());
			$db->q("UPDATE PAGE_ELEMENT SET TOPLEFTX = ".$xmin." WHERE ID = ".$coordinates['ID']." AND TOPLEFTX IS NULL",array());
			$db->q("UPDATE PAGE_ELEMENT SET TOPLEFTY = ".$ymin." WHERE ID = ".$coordinates['ID']." AND TOPLEFTY IS NULL",array());
			
			$boxes[$coordinates['NAME']][$coordinates['ID']] = array(
							 'left'   => $xmin,
							 'top'    => $ymin,
							 'width'  => ($xmax - $xmin),
							 'height' => ($ymax - $ymin),
							 'name'   => "Mask " . (count($boxes[$coordinates['NAME']])+1),
							 'mask'   => $element_data_type
					   );
		
		//}
	
	}while($coordinates = $db->fetch_array($coordQuery));
	
	$count = 0;
	$colors = array('"rgb(100,250,50)"','"rgb(212,144,27)"','"rgb(149,82,222)"','"rgb(0,168,255)"');
	foreach($pixels as $datatype => $pixelmask):
		$identifier = $pageimage ."-". $datatype;
		makePixelMask($identifier,$path,$width,$height,$pixelmask,175,$colors[$count%4]);
		$count++;
	endforeach;
	
	//makePixelMask($pageimage,$path,$width,$height,$aggregateMask);
	
	return $boxes;
}
/**
 * Takes an LOB, loads it and parses it into an array of x and y coordinates
 * 
 * @param [in] $maskLOB - oracle large object containing a string of pixel pairs
 *
 * @return - an array with two sub arrays ['x'] and ['y'] which contain the x y pairs
 *			 at matching keys
 *
 */
function convert_db_mask_to_array($maskLOB)
{
	$coordinates = trim($maskLOB->load());
	
	$coordinates = substr($coordinates,1,strlen($coordinates)-1);
	//Remove first and last parenthesis

	$coordinates = explode(')(',$coordinates);
	//break coords into comma separated xy pairs
	
	foreach($coordinates as $coord):
	
		$pair = explode(",",$coord);
		$coords['x'][] = str_replace("(","",$pair[0]);
		$coords['y'][] = str_replace(")","",$pair[1]); 
		//turn comma separated pairs into int array
		
	endforeach;
	
	return $coords;
}
/**
 * Gets the min and max for the x and y of a pixel mask
 * 
 * @param [in] $coords - the pixel array for a pixel mask in the form ['x'] and ['y'] with pairs at
 *						 matching keys for the x and y arrays
 * 
 * return - an array containing the x and y max and min
 */
function getMinMaxCoords($coords)
{
	$out['xmin'] = min($coords['x']);
	$out['ymin'] = min($coords['y']);
	$out['xmax'] = max($coords['x']);
	$out['ymax'] = max($coords['y']);
	
	return $out;
}
/**
 * Generates a transparent pixelmask for the \a page_element of which its \a id is given.
 * 
 * @param [in] $id the data item id for this mask
 * @param [in] $path the location to create mask
 * @param [in] $width width of mask image
 * @param [in] $height height of mask image
 * @param [in] $pixels array of pixels marked as 0 or 255
 * @param [in] $intensity intensity setting for ImageMagick
 * @param [in] $color color setting for ImageMagick
 * 
 */
function makePixelMask($id,$path,$width,$height,$pixels,$intensity = 175,$color = '"rgb(100,250,50)"')
{

	$filename = $path."/$id".'_alpha.txt';
	$output = fopen($filename,'w');
	
	// Using ImageMagick text format for initial output
	fwrite($output,"# ImageMagick pixel enumeration: $width,$height,255,rgb\n");
	
	$index = 0;
	$size = $width*$height;
	for($pixel = 0;$pixel < $size;$pixel++):
		$x = $pixel % $width;
		$y = intval($pixel / $width);
		
		fwrite($output,"$x,$y: ".($pixels[$pixel] ? "($intensity,$intensity,$intensity)  #" .dechex($intensity).dechex($intensity).dechex($intensity).'  grey':'(  0,  0,  0)  #000000  black'). "\n" );
		$index++;
	endfor;
	
	fwrite($output,"\n");
	fclose($output);
	
	$outfile = $path."/".$id."_alpha.png";
	
	return(system("/opt/csw/bin/convert $filename -background $color -alpha shape $outfile"));
}
/**
 * This function returns a json encoding of all page elements for a given page image
 *  
 * @param [in] $dataitem the page image to find elements for
 *
 * @return json encoded list of page elements and their boundaries relative to the fullsize
 *		 	 image.
 */
function get_page_element_boxes($dataitem)
{
	global $db;
	
	settype($dataitem, 'integer');
	$elements = $db->q("SELECT PAGE_ELEMENT.ID, PAGE_ELEMENT.WIDTH, PAGE_ELEMENT.HEIGHT, TOPLEFTX AS X, TOPLEFTY AS Y, D.NAME as ELEMENTCATEGORY, DATATYPE_PROPERTY.NAME AS KIND
						FROM PAGE_ELEMENT JOIN CONTAINS_PAGE_ELEMENT C ON C.PAGE_ELEMENT_ID = PAGE_ELEMENT.ID 
						LEFT JOIN ASSOCIATE_DATATYPE_DATA_ITEM ON DATA_ITEM_ID = PAGE_ELEMENT.ID 
						LEFT JOIN DATATYPE D ON DATATYPE_ID = D.ID
						LEFT JOIN HAS_VALUE H ON H.PAGE_ELEMENT_ID = PAGE_ELEMENT.ID
						LEFT JOIN VALUE_OF V ON V.PAGE_ELEMENT_PROPERTY_VALUE_ID = H.PAGE_ELEMENT_PROPERTY_VALUE_ID
						LEFT JOIN DATATYPE_PROPERTY ON V.DATA_TYPE_PROPERTY_ID = DATATYPE_PROPERTY.ID
						WHERE PAGE_IMAGE_ID = $dataitem 
						AND TOPLEFTY IS NOT NULL 
						AND TOPLEFTX IS NOT NULL 
						AND WIDTH IS NOT NULL 
						AND HEIGHT IS NOT NULL 
						AND MASK IS NULL
						ORDER BY ELEMENTCATEGORY, KIND, ID",array());
	$i=0;
	
	if(!isset($page_elements))
		$page_elements = array();
		
	while($element = $db->fetch_array($elements)):
	
		$type = ($element['ELEMENTCATEGORY']?$element['ELEMENTCATEGORY']:"Miscellaneous");
		
		if(empty($element['KIND']))
			$element['KIND'] = "Element";

		if(!isset($count[$element['ELEMENTCATEGORY']][$element['KIND']]))
			$count[$element['ELEMENTCATEGORY']][$element['KIND']] = 1;
		else
			$count[$element['ELEMENTCATEGORY']][$element['KIND']]++;
		
		$coords['name'] = $element['KIND'] ." ". $count[$element['ELEMENTCATEGORY']][$element['KIND']];
		$coords['left'] = $element['X'];
		$coords['top'] = $element['Y'];
		$coords['height'] = $element['HEIGHT'];
		$coords['width'] = $element['WIDTH'];
		
		$page_elements[$type][$element['ID']] = $coords;
		$i++;

	endwhile;
	
	$pixel_mask_boxes = generate_pixelmask($dataitem);
	
	$page_elements = array_merge($pixel_mask_boxes,$page_elements);
	
	if(empty($page_elements))
		$page_elements = "No Elements Exist";
	
	return json_encode($page_elements);
	
}

/**
 * This function returns a json listing of datatypes matching
 * a search query given by GET term
 *  
 * @param $_GET['term']
 *
 * @return json search results
 */
function dae_data_page_element_type_search(){

	$query = check_plain($_GET['term']);
	
	$db = getDAEDatabase();
	 
	$q= "SELECT ID, NAME, ROW_NUMBER() over(ORDER BY ID) as r  
		 FROM DATATYPE
		 WHERE lower(NAME) like '".strtolower($query)."%'
		 ORDER BY NAME";
	 
	$types = $db->q($q,array());
					
 	$i = 0;
	$typelist = array();
	$exists = array();
	while($row = $db->fetch_array($types)):
		
		if(!in_array($row['NAME'],$exists)){
			$typelist[$i]['id'] = $row['ID'];
			$typelist[$i]['label'] = $row['NAME'];
			$typelist[$i]['value'] = $row['NAME'];
			$exists[] = $row['NAME'];
			$i++;
		}
				
	endwhile;
	
	drupal_json($typelist);


}


/**
 * This function generates the png alpha mask for pixel masks
 *
 *  
 * @param [in] $request an array containing ['ID'] = the page image id
 *
 * @return array of bounding boxes for masks by data item id (todo)
 */
function dae_pageimage_page_generate_pixelmask($dataitem)
{
	generate_pixelmask($dataitem);
}

function dae_data_pageimage_load_mask($pageimage)
{
	if(is_numeric($pageimage))
		echo get_page_element_boxes($pageimage);
	else
		echo "Bad Data Item ID"; 
}
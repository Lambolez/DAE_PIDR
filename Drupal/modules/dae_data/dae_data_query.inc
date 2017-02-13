<?php

require_once("./".drupal_get_path('module','daedatabase')."/daedatabase_db.inc");
require_once("./".drupal_get_path('module','dae_data')."/dae_data_query_client.inc");

/*
 * 
 * select (all or n random) page images
 * having (any or some type)
 * contained in (any or some datasets)
 * containing page_elements of (any or some type)
 * and/or having a datatype_property of (any or some type)
 * and/or containing interpretations of value (XXX - free form edit)
 */
function query_page(){
	drupal_add_js(drupal_get_path('module','dae_ui')."/javascript/data_query.js");
	drupal_add_css(drupal_get_path('module','dae_ui')."/data_query.css");
	$str="<form>";
	$str=$str."<p>SELECT ";
	$str=$str."<input type=\"hidden\" id=\"v_count\" name=\"v_count\" value=\"1000\"/>".
		"<a class=\"query_parameter\" id=\"a_count\" title=\"click to add count limit\" onclick=\"change_count()\"></a>";
	$str=$str." page images that: </p>";
	$str=$str."<ul class=\"normal\">";
	
	//type
	$str=$str."<li><input type=\"hidden\" id=\"l_pitype\" name=\"l_pitype\" value=\"1\"/>".
		"<input type=\"hidden\"  id=\"tl_pitype\" value=\"does not have|has\"/>".
		"<a class=\"logic\" id=\"al_pitype\" onclick=\"changeLogic('l_pitype','al_pitype','tl_pitype')\"></a> ".
		"type: <input type=\"hidden\" id=\"v_pitype\" name=\"v_pitype\" value=\"[]\"/>".
		"<a class=\"query_parameter\" id=\"a_pitype\" title=\"click to add type\" onclick=\"adding_vect('v_pitype','a_pitype','div_pitype')\"></a>".
		"<div id=\"div_pitype\"></div></li>";
	
	//dataset
	$str=$str."<li><input type=\"hidden\" id=\"l_dataset\" name=\"l_dataset\" value=\"1\"/>".
		"<input type=\"hidden\" id=\"tl_dataset\" value=\"is not|is\"/>".
		"<a class=\"logic\" id=\"al_dataset\" onclick=\"changeLogic('l_dataset','al_dataset','tl_dataset')\"></a> ".
		"in dataset: <input type=\"hidden\" id=\"v_dataset\" name=\"v_dataset\" value=\"[]\"/>".
		"<a class=\"query_parameter\" id=\"a_dataset\" title=\"click to add dataset\" onclick=\"adding_vect('v_dataset','a_dataset','div_dataset')\"></a>".
		"<div id=\"div_dataset\"></div></li>";
	
	//page element type
	$str=$str."<li><input type=\"hidden\" id=\"l_petype\" name=\"l_petype\" value=\"1\"/>".
		"<input type=\"hidden\"  id=\"tl_petype\" value=\"does not contain|contains\"/>".
		"<a class=\"logic\" id=\"al_petype\" onclick=\"changeLogic('l_petype','al_petype','tl_petype')\"></a> ".
		"page elements of type: <input type=\"hidden\" id=\"v_petype\" name=\"v_petype\" value=\"[]\"/>".
		"<a class=\"query_parameter\" id=\"a_petype\" title=\"click to add type\" onclick=\"adding_vect('v_petype','a_petype','div_petype')\"></a>".
		"<div id=\"div_petype\"></div></li>";

	//page element datatype_property
	$str=$str."<li><input type=\"hidden\" id=\"l_pedtprop\" name=\"l_pedtprop\" value=\"1\"/>".
		"<input type=\"hidden\"  id=\"tl_pedtprop\" value=\"does not contain|contains\"/>".
		"<a class=\"logic\" id=\"al_pedtprop\" onclick=\"changeLogic('l_pedtprop','al_pedtprop','tl_pedtprop')\"></a> ".
		"page elements having datatype property: <input type=\"hidden\" id=\"v_pedtprop\" name=\"v_pedtprop\" value=\"[]\"/>".
		"<a class=\"query_parameter\" id=\"a_pedtprop\" title=\"click to add datatype property\" ".
		"onclick=\"adding_vect('v_pedtprop','a_pedtprop','div_pedtprop')\"></a>".
		"<div id=\"div_pedtprop\"></div></li>";

	//page element value
	$str=$str."<li>".
		"contains page elements having value: <input type=\"hidden\" id=\"v_peval\" name=\"v_peval\" value=\"\"/>".
		"<a class=\"query_parameter\" id=\"a_peval\" title=\"click to add value\" ".
		"onclick=\"adding_val('v_peval','a_peval','div_peval')\"></a>".
		"<div id=\"div_peval\"></div></li>";
	
	$str=$str."</ul>";
	
	$str=$str."<input type=\"button\" value=\"Submit Query\" onclick=\"query_now()\"";
	
	$str=$str."</form><script>count_text();".
		"vect_text('v_pitype','a_pitype','div_pitype');".
		"vect_text('v_dataset','a_dataset','div_dataset');".
		"vect_text('v_petype','a_petype','div_petype');".
		"vect_text('v_pedtprop','a_pedtprop','div_pedtprop');".
		"logi_text('l_pitype','al_pitype','tl_pitype');".
		"logi_text('l_dataset','al_dataset','tl_dataset');".
		"logi_text('l_petype','al_petype','tl_petype');".
		"logi_text('l_pedtprop','al_pedtprop','tl_pedtprop');".
		"val_text('v_peval','a_peval','div_peval');</script>";
	return $str;
}

function query_form(){
	$dn=arg(2);
	$val=arg(3);
	$cur_div=-1;
	$errflag=false;
	if($dn=="div_pitype"){
		$vtag="v_pitype";
		$atag="a_pitype";
		$cur_div=0;
	}else if($dn=="div_dataset"){
		$vtag="v_dataset";
		$atag="a_dataset";
		$cur_div=1;
	}else if($dn=="div_petype"){
		$vtag="v_petype";
		$atag="a_petype";
		$cur_div=2;
	}else if($dn=="div_pedtprop"){
		$vtag="v_pedtprop";
		$atag="a_pedtprop";
		$cur_div=3;
	}else{
		$errflag=true;
	}
	
	if($errflag){
		echo "alert('Unknown div: $dn, val: $val');"; 
	}else{
		$varr=preg_split('/\\|/',$val);
		$arrarr=array();
		$arrarr[0]=idstr2arr($varr[0]);
		$arrarr[1]=idstr2arr($varr[1]);
		$arrarr[2]=idstr2arr($varr[2]);
		$arrarr[3]=idstr2arr($varr[3]);
		$farr=array();
		$farr[0]=$varr[4];
		$farr[1]=$varr[5];
		$farr[2]=$varr[6];
		$farr[3]=$varr[7];
		
		
		if(count($arrarr[$cur_div])>0){
			$html='<ul>';
			for($i=0;$i<=count($arrarr[$cur_div])-1;$i++){
				$curid=$arrarr[$cur_div][$i];
				if($curid=='-1'){
					$html=$html.sel2($dn,$arrarr,$farr,$cur_div,$i);
				}else{
					$nm=getNameById($dn,$curid);
					$des=getDescById($dn,$curid);
					$html=$html."<li><a class=\"query_text\" onclick=\"jAlert(\'$des\',\'About: $nm\')\">$nm</a>".
						" <a class=\"query_remove\" title=\"click to remove\" onclick=\"remove_vect(\'$vtag\',\'$atag\',\'$dn\',$i)\">X</a></li>";
				}
			}
			$html=$html.'</ul>';
		}else{
			$html='';
		}
		echo "$('#$dn').html('$html');";
	}
}

function sel2($dn,$arrarr,$farr,$ai,$ci){
	$db = getDAEDatabase();
	$rarr=array();
	$rarr[0]=$arrarr[0];
	$rarr[1]=$arrarr[1];
	$rarr[2]=$arrarr[2];
	$rarr[3]=$arrarr[3];
	$rarr[4]=$arrarr[4];
	
	settype($ai,"integer");
	$rarr[$ai]=array();
	
	$subsql=genSQL(0,$rarr[0],$rarr[1],$rarr[2],$rarr[3],array(),$farr[0],$farr[1],$farr[2],$farr[3]);
	
	if($dn=='div_pitype'){
		$vtag="v_pitype";
		$atag="a_pitype";
		$tmptag="tmp_pitype";
		$sql="select distinct dt.ID,dt.NAME,dt.DESCRIPTION from associate_datatype_data_item addi, datatype dt ".
			"where addi.DATATYPE_ID=dt.ID and addi.DATA_ITEM_ID in ($subsql) order by dt.NAME";
	}else if($dn=='div_dataset'){
		$vtag="v_dataset";
		$atag="a_dataset";
		$tmptag="tmp_dataset";
		$sql="select distinct dataset.ID,NAME,DESCRIPTION from dataset, data_item where ". 
			"dataset.ID in (".
				"(select distinct(DATASET_ID) from associate_dataset ". 
				"start with ASSOCIATING_DATASET_ID in (select distinct(ipi.DATASET_ID) ".
				"from includes_page_image ipi where ipi.PAGE_IMAGE_ID in ($subsql) ) connect by prior DATASET_ID = ASSOCIATING_DATASET_ID) ". 
  				"union (select distinct(DATASET_ID) from includes_page_image )) ".
  			"and dataset.ID=data_item.ID order by NAME";
	}else if($dn=='div_petype'){
		$vtag="v_petype";
		$atag="a_petype";
		$tmptag="tmp_petype";
		$sql="select distinct dt.ID,dt.NAME,dt.DESCRIPTION from contains_page_element cpe, associate_datatype_data_item addi, datatype dt ".
			"where addi.DATATYPE_ID=dt.ID and cpe.PAGE_ELEMENT_ID=addi.DATA_ITEM_ID and cpe.PAGE_IMAGE_ID in ($subsql) order by dt.NAME";
	}else if($dn=='div_pedtprop'){
		$vtag="v_pedtprop";
		$atag="a_pedtprop";
		$tmptag="tmp_pedtprop";
		$sql="select distinct dp.ID,dp.NAME,dp.DESCRIPTION from datatype_property dp, value_of vo, has_value hv, contains_page_element cpe ".
			"where dp.ID=vo.DATA_TYPE_PROPERTY_ID and vo.PAGE_ELEMENT_PROPERTY_VALUE_ID=hv.PAGE_ELEMENT_PROPERTY_VALUE_ID ".
			"and hv.PAGE_ELEMENT_ID=cpe.PAGE_ELEMENT_ID and cpe.PAGE_IMAGE_ID in ($subsql) order by dp.NAME";
	}
	$pairs1=array();
	$pairs2=array();
	$result=$db->q($sql,array());
	while($row = $db->fetch_array($result)){
		$pairs1[$row['ID']]=$row['NAME'];
		$pairs2[$row['ID']]=$row['DESCRIPTION'];
	}
	foreach($arrarr[$ai] as $ban){
		unset($pairs1[$ban]);
		unset($pairs2[$ban]);
	}
	$rslt="<li><select id=\"$tmptag\" onChange=\"selChange(\'$tmptag\')\" onkeyup=\"selChange(\'$tmptag\')\">";
	foreach(array_keys($pairs1) as $key){
		$hstr=$pairs2[$key];
		$hstr=str_replace("'","\\'",$hstr);
		$hstr=preg_replace('/\s+/',' ',$hstr);
		$rslt=$rslt."<option value=\"$key\" title=\"$hstr\">".$pairs1[$key]."</option>";
	}
	$rslt=$rslt."</select> <a class=\"query_add\" onclick=\"added_vect(\'$vtag\',\'$atag\',\'$dn\',\'$tmptag\')\">Add</a> ".
		" <a class=\"query_remove\" title=\"click to remove\" onclick=\"remove_vect(\'$vtag\',\'$atag\',\'$dn\',$ci)\">X</a>".
		"<div class=\"query_hint\" id=\"hint_$tmptag\"/></li>";
	return $rslt;
}

function sel1($dn,$szs,$ci){
	$db = getDAEDatabase();
	if($dn=='div_pitype'){
		$vtag="v_pitype";
		$atag="a_pitype";
		$tmptag="tmp_pitype";
		$sql="select distinct dt.ID,dt.NAME,dt.DESCRIPTION from page_image pi, associate_datatype_data_item addi, datatype dt ".
			"where addi.DATATYPE_ID=dt.ID and pi.ID=addi.DATA_ITEM_ID";
	}else if($dn=='div_dataset'){
		$vtag="v_dataset";
		$atag="a_dataset";
		$tmptag="tmp_dataset";
		$sql="select distinct dataset.ID,NAME,DESCRIPTION from dataset, data_item where ". 
			"dataset.ID in (".
				"(select distinct(DATASET_ID) from associate_dataset ". 
				"start with ASSOCIATING_DATASET_ID in (select distinct(ipi.DATASET_ID) ".
				"from includes_page_image ipi) connect by prior DATASET_ID = ASSOCIATING_DATASET_ID) ". 
  				"union (select distinct(DATASET_ID) from includes_page_image )) ".
  			"and dataset.ID=data_item.ID";
	}else if($dn=='div_petype'){
		$vtag="v_petype";
		$atag="a_petype";
		$tmptag="tmp_petype";
		$sql="select distinct dt.ID,dt.NAME,dt.DESCRIPTION from page_element pe, associate_datatype_data_item addi, datatype dt ".
			"where addi.DATATYPE_ID=dt.ID and pe.ID=addi.DATA_ITEM_ID";
	}else if($dn=='div_pedtprop'){
		$vtag="v_pedtprop";
		$atag="a_pedtprop";
		$tmptag="tmp_pedtprop";
		$sql="select distinct dp.ID,dp.NAME,dp.DESCRIPTION from datatype_property dp, value_of vo, has_value hv, page_element pe ".
			"where dp.ID=vo.DATA_TYPE_PROPERTY_ID and vo.PAGE_ELEMENT_PROPERTY_VALUE_ID=hv.PAGE_ELEMENT_PROPERTY_VALUE_ID and hv.PAGE_ELEMENT_ID=pe.ID";
	}
	$pairs1=array();
	$pairs2=array();
	$result=$db->q($sql,array());
	while($row = $db->fetch_array($result)){
		$pairs1[$row['ID']]=$row['NAME'];
		$pairs2[$row['ID']]=$row['DESCRIPTION'];
	}
	foreach($szs as $ban){
		unset($pairs1[$ban]);
		unset($pairs2[$ban]);
	}
	$rslt="<li><select id=\"$tmptag\" onChange=\"selChange(\'$tmptag\')\" onkeyup=\"selChange(\'$tmptag\')\">";
	foreach(array_keys($pairs1) as $key){
		$hstr=$pairs2[$key];
		$hstr=str_replace("'","\\'",$hstr);
		$hstr=preg_replace('/\s+/',' ',$hstr);
		$rslt=$rslt."<option value=\"$key\" title=\"$hstr\">".$pairs1[$key]."</option>";
	}
	$rslt=$rslt."</select> <a class=\"query_add\" onclick=\"added_vect(\'$vtag\',\'$atag\',\'$dn\',\'$tmptag\')\">Add</a> ".
		" <a class=\"query_remove\" title=\"click to remove\" onclick=\"remove_vect(\'$vtag\',\'$atag\',\'$dn\',$ci)\">X</a>".
		"<div class=\"query_hint\" id=\"hint_$tmptag\"/></li>";
	return $rslt;
}

function getNameById($dn,$id){
	$db = getDAEDatabase();
	settype($id, 'integer');
	if($dn=='div_pitype'||$dn=='div_petype'){
		$sql="select NAME from datatype where ID=$id";
	}else if($dn=='div_dataset'){
		$sql="select NAME from dataset where ID=$id";
	}else if($dn=='div_pedtprop'){
		$sql="select NAME from datatype_property where ID=$id";
	}
	$hstr=$db->r($sql,array());
	/**
	 * @BUG
	 * The following lines of code are actually useless and appropriate
	 * characters in the retrieved string will still mess up javasctipt!
	 */
	$hstr=str_replace("'","\\'",$hstr);
	$hstr=str_replace("(","\\(",$hstr);
	$hstr=str_replace(")","\\)",$hstr);
	$hstr=preg_replace('/\s+/',' ',$hstr);
	return $hstr;
}

function getDescById($dn,$id){
	$db = getDAEDatabase();
	settype($id, 'integer');
	if($dn=='div_pitype'||$dn=='div_petype'){
		$sql="select DESCRIPTION from datatype where ID=$id";
	}else if($dn=='div_dataset'){
		$sql="select DESCRIPTION from data_item where ID=$id";
	}else if($dn=='div_pedtprop'){
		$sql="select DESCRIPTION from datatype_property where ID=$id";
	}
	$hstr=$db->r($sql,array());
	$hstr=str_replace("'","\\'",$hstr);
	$hstr=str_replace("(","\\(",$hstr);
	$hstr=str_replace(")","\\)",$hstr);
	$hstr=preg_replace('/\s+/',' ',$hstr);
	return $hstr;
}

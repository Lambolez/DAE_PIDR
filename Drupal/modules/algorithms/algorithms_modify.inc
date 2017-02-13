<?php
require_once("./".drupal_get_path('module','daedatabase')."/daedatabase_db.inc");
require_once("./".drupal_get_path('module','dae_data')."/dae_data_person.inc");

function modify_algorithm_ui(){
	drupal_add_js(drupal_get_path('module','dae_ui')."/javascript/algorithm_modify.js");
	$str="<form action=\"./?q=algorithm/submit_modification/\" method=\"POST\" onsubmit=\"return form_check()\" enctype=\"multipart/form-data\"><div id=\"step1\"></div><div id=\"step2\"></div>";
	
	$str=$str."<div id=\"step3\"></div><div id=\"step4\"></div><div id=\"step5\"></div><div id=\"step6\"></div></form><div id=\"progressbar\"></div>";
	$str=$str."<script>step(0,0,true);</script>";
	return $str;
}


function modify_form(){
	$step_num=arg(2);
	$val=arg(3);
	//$did="#step".$step_num;
	$next_step=formContent($step_num,$val);
	
	echo $next_step;
}

function formContent($sn,$val){
	if($sn=='0'){
	// initial status
		$id='#step1';
		$content="<input type=\"radio\" name=\"modify_type\" value=\"create\" onChange=\"step(1,\'create\',true)\" />Create a new algorithm ".
			"<input type=\"radio\" name=\"modify_type\" value=\"upgrade\" onChange=\"step(1,\'upgrade\',true)\" /> Upgrade an existing algorithm";
		$ns="$('$id').html('$content');";
	}else if($sn=='1'){
		$id='#step2';
		if($val=='create'){
			$content="Algorithm Name: <input type=\"text\" id=\"alg_name\" name=\"alg_name\"/> <input type=\"button\" value=\"next\" onClick=\"step(2,\'-1\',true)\" />";
		}else{
			$content="Select Algorithm: <select id=\"select_aid\" name=\"aid\">";
			$algs=getAllAlgIdName();
			foreach($algs as $alg){
    			$content=$content."<option value=\"".$alg['id']."\">".$alg['name']."</option>";
			}
			$content=$content."</select> <input type=\"button\" value=\"next\" onClick=\"step(2,getSelect(),true)\" />".
						"<input type=\"hidden\" id=\"alg_name\" name=\"alg_name\"/>";
		}
		$ns="$('$id').html('$content');";
	}else if($sn=="2"){
		$id='#step3';
		$content=basicInfo($val);
		$ns="";
		if($val!=-1){
			$name=getAlgName($val);
			$ns="document.getElementById('alg_name').value='$name';";
		}
		$ns=$ns."$('$id').html('$content');step(3,'$val',true);step(4,'$val',true);step(5,0,true);";
	}else if($sn=="3"){
		$id='#step4';
		$content=IOpart($val,$sn);
		$ns="$('$id').html('$content');";
	}else if($sn=="4"){
		$id='#step5';
		$content=IOpart($val,$sn);
		$ns="$('$id').html('$content');";
	}else if($sn=="5"){
		$id='#step6';
		$content="<input type=\"submit\" value=\"submit\">";
		$ns="$('$id').html('$content');";
	}else if ($sn=="100"){
		// for checking version existing
		$sp="/___/";
		$szs=preg_split($sp,$val);
		$an=$szs[0];
		$av=$szs[1];
		$ns=getExists($an,$av);
	}else{
		$ns="alert('sn=$sn, val=$val');";
	}
	return $ns;
}
function getExists($aname,$aversion){
	$db = getDAEDatabase();
	$sql="select count(*) from algorithm where NAME=:aname and VERSION=:aversion";
	$variables[':aname'] = $aname;
	$variables[':aversion'] = $aversion;
	return $db->r($sql,$variables);
}
function getAlgName($aid){
	$db = getDAEDatabase();
	settype($aid, 'integer');
	$sql="select NAME from algorithm where ID=$aid";
	return $db->r($sql,array());
}

function IOpart($val,$sn){
	$title="NA";
	if($sn=="3"){
		$title="Input";
	}else if($sn=="4"){
		$title="Output";
	}
	$str="<h4>$title</h4>";
	$sp="/@@@/";
	$szs=preg_split($sp,$val);
	// $val = $aid $sp $line $sp $line ....
	$aid=$szs[0];
	settype($aid, 'integer');
	$str=$str."<table><tr><th>Name</th><th>Description</th><th>Type</th><th>Ordering</th><th>Required</th><th></th></tr>";
	$db = getDAEDatabase();
	if(count($szs)==1){
		$sql="select count(*) from algorithm_$title aio, data, datatype dt, type_of ".
			"where aio.DATA_ID=data.ID and data.ID=type_of.DATA_ID and type_of.DATATYPE_ID=dt.ID and aio.ALGORITHM_ID=$aid ";
		$ttl=$db->r($sql,array());
		$sql="select aio.DATA_ID as DID, data.NAME as DNAME, data.DESCRIPTION as DDES, dt.ID as DTID, data.ORDERING as DORDER, data.REQUIRED as DREQ, dt.NAME as DTNAME, dt.DESCRIPTION as DTDES ".
			"from algorithm_$title aio, data, datatype dt, type_of ".
			"where aio.DATA_ID=data.ID and data.ID=type_of.DATA_ID and type_of.DATATYPE_ID=dt.ID and aio.ALGORITHM_ID=$aid ".
			"order by DORDER";
		$result=$db->q($sql,array());
		$cnt=0;
		while($row = $db->fetch_array($result)){
			$cnt++;
			$did=$row['DID'];
			$name=$row['DNAME'];
			$order=$row['DORDER'];
			$req=$row['DREQ'];
			$des=$row['DDES'];
			$name2=$row['DTNAME'];
			$des2=$row['DTDES'];
			$dtid=$row['DTID'];
			$rstr="No";
				if($req=="1"){
					$rstr="Yes";
				}
			$str=$str."<tr><td>$name</td><td>$des</td><td>$name2</td>"
				."<td>$order</td><td>$rstr</td>"
				."<td><input type=\"button\" value=\"edit\" onclick=\"edit($aid,$cnt,$ttl,$sn)\"/>".
				"<input type=\"button\" value=\"remove\" onclick=\"remove($aid,$cnt,$ttl,$sn)\"/></td></tr>";
			$str=$str."<input type=\"hidden\" name=\"name_$sn"."_$cnt\" id=\"name_$sn"."_$cnt\" value=\"$name\" />".
				"<input type=\"hidden\" name=\"des_$sn"."_$cnt\" id=\"des_$sn"."_$cnt\" value=\"$des\" />".
				"<input type=\"hidden\" name=\"dtid_$sn"."_$cnt\" id=\"dtid_$sn"."_$cnt\" value=\"$dtid\" />".
				"<input type=\"hidden\" name=\"req_$sn"."_$cnt\" id=\"req_$sn"."_$cnt\" value=\"$req\" />".
				"<input type=\"hidden\" name=\"flag_$sn"."_$cnt\" id=\"flag_$sn"."_$cnt\" value=\"0\" />";
		}
		$str=$str."<tr><td><input type=\"button\" value=\"add\" onclick=\"add($aid,$ttl,$sn)\"/></td></tr>";
		$str=$str."</table>";
		$str=$str."<input type=\"hidden\" name=\"total_$sn\" value=\"$ttl\"/>";
	}else{
		$ttl=(count($szs)-1);
		for($cnt=1;$cnt<=count($szs)-1;$cnt++){
			$line=$szs[$cnt];
			// $line = $flag $sp2 $name $sp2 $des $sp2 $dtid $sp2 $req
			$sp2="/!!!/";
			$cols=preg_split($sp2,$line);
			$flag=$cols[0];
			$name=$cols[1];
			$des=$cols[2];
			$dtid=$cols[3];
			$req=$cols[4];
			if(!empty($dtid)){
				$sql="select NAME from datatype where ID=$dtid";
				settype($dtid, 'integer');
				$name2=$db->r($sql,array());
			}else{
				$name2="";
			}
			if($flag==1){ // $flag==1 means edit mode
				$rstr="";
				if($req=="1"){
					$rstr="checked=\"checked\"";
				}
				$str=$str."<tr><td><input type=\"text\" name=\"name_$sn"."_$cnt\" id=\"name_$sn"."_$cnt\" value=\"$name\" /></td>".
					"<td><input type=\"text\" name=\"des_$sn"."_$cnt\" id=\"des_$sn"."_$cnt\" value=\"$des\" /></td>".
					"<td><input type=\"text\" name=\"name2_$sn"."_$cnt\" value=\"$name2\" readonly onfocus=\"chooseType($aid,$cnt,$ttl,$sn)\"/>".
					"<input type=\"hidden\" name=\"dtid_$sn"."_$cnt\" id=\"dtid_$sn"."_$cnt\" value=\"$dtid\" /></td>"
					."<td>$cnt<input type=\"button\" value=\"&uArr;\" onclick=\"moveUp($aid,$cnt,$ttl,$sn)\"/>"
					."<input type=\"button\" value=\"&dArr;\" onclick=\"moveDown($aid,$cnt,$ttl,$sn)\"/></td>"
					."<td><input type=\"checkbox\" name=\"req_$sn"."_$cnt\" id=\"req_$sn"."_$cnt\" value=\"1\" $rstr/></td>"
					."<td><input type=\"button\" value=\"remove\" onclick=\"remove($aid,$cnt,$ttl,$sn)\"/></td></tr>"
					."<input type=\"hidden\" name=\"flag_$sn"."_$cnt\" id=\"flag_$sn"."_$cnt\" value=\"1\" />";
			}else{ // display mode
				$rstr="No";
				if($req=="1"){
					$rstr="Yes";
				}
				$str=$str."<tr><td>$name</td><td>$des</td><td>$name2</td>"
					."<td>$cnt</td><td>$rstr</td>"
					."<td><input type=\"button\" value=\"edit\" onclick=\"edit($aid,$cnt,$ttl,$sn)\"/>".
					"<input type=\"button\" value=\"remove\" onclick=\"remove($aid,$cnt,$ttl,$sn)\"/></td></tr>";
				$str=$str."<input type=\"hidden\" name=\"name_$sn"."_$cnt\" id=\"name_$sn"."_$cnt\" value=\"$name\" />".
					"<input type=\"hidden\" name=\"des_$sn"."_$cnt\" id=\"des_$sn"."_$cnt\" value=\"$des\" />".
					"<input type=\"hidden\" name=\"dtid_$sn"."_$cnt\" id=\"dtid_$sn"."_$cnt\" value=\"$dtid\" />".
					"<input type=\"hidden\" name=\"req_$sn"."_$cnt\" id=\"req_$sn"."_$cnt\" value=\"$req\" />".
					"<input type=\"hidden\" name=\"flag_$sn"."_$cnt\" id=\"flag_$sn"."_$cnt\" value=\"0\" />";
			}
		}
		$str=$str."<tr><td><input type=\"button\" value=\"add\" onclick=\"add($aid,$ttl,$sn)\"/></td></tr>";
		$str=$str."</table>";
		$str=$str."<input type=\"hidden\" name=\"total_$sn\" value=\"$ttl\"/>";
	
	}
	return $str;
}

function basicInfo($aid){
	$id=$aid;
	$version="";
	$description="";
	$timec="";
	$spacec="";
	if($id!='-1'){
		$db = getDAEDatabase();
		settype($id, 'integer');
		$sql="select VERSION,DESCRIPTION,TIME_COMPLEXITY,SPACE_COMPLEXITY from algorithm where ID=".$id;
		$result=$db->q($sql,array());
		if(count($result)==1){
			$row = $db->fetch_array($result);
			$version=$row['VERSION'];
			$description=$row['DESCRIPTION'];
			$timec=$row['TIME_COMPLEXITY'];
			$spacec=$row['SPACE_COMPLEXITY'];
		}
	}
	$str="<table>";
	$str=$str."<tr><td>ID: </td><td><input type=\"text\" id=\"alg_id\" name=\"alg_id\" value=\"$id\" readonly/></td></tr>";
	$str=$str."<tr><td>Version: </td><td><input type=\"text\" id=\"alg_version\" name=\"alg_version\"  value=\"$version\"/></td></tr>";
	$str=$str."<tr><td>Description: </td><td><input type=\"text\" id=\"alg_description\" name=\"alg_description\"  value=\"$description\"/></td></tr>";
	$str=$str."<tr><td>Time Complexity: </td><td><input type=\"text\" id=\"alg_timecomplexity\" name=\"alg_timecomplexity\"  value=\"$timec\"/></td></tr>";
	$str=$str."<tr><td>Space Complexity: </td><td><input type=\"text\" id=\"alg_spacecomplexity\" name=\"alg_spacecomplexity\"  value=\"$spacec\"/></td></tr>";
	$str=$str."<tr><td>File: </td><td><input type=\"file\" id=\"alg_file\" name=\"alg_file\" /></td></tr>";
	$str=$str."</table>";
	$str=$str."<input type=\"hidden\" id=\"ct_tmp\"";
	return $str;
}
function cleanStepsAfter($sn){
	
}
function getAllAlgIdName(){
	$all=array();
	$db = getDAEDatabase();
	$sql="select ID,NAME from algorithm";
	$result=$db->q($sql,array());
	while($row = $db->fetch_array($result)){
		$id=$row['ID'];
		$name=$row['NAME'];
		array_push($all,array('id'=>$id,'name'=>$name));
	}
	return $all;
}

function choose_type(){
	$step_num=arg(2);
	$val=arg(3);
	$next_step=ct_content($step_num,$val);
	echo $next_step;
}

function ct_content($sn,$val){
	$ns="";
	$content="undefined";
	$id="ct_div".$sn;
	if($sn=='1'){
	// initial status
		if($val=='-1'){
			$content="<input type=\"radio\" name=\"modify_type\" value=\"create\" onChange=\"ct_div(2,-1)\" />Create a new data type <br/>".
			"<input type=\"radio\" name=\"modify_type\" value=\"upgrade\" onChange=\"ct_div(2,-2)\" /> select an existing data type";
			$ns="document.getElementById('$id').innerHTML='$content';".
				"document.getElementById('ct_div2').innerHTML='';";
		}else{
			$content="<input type=\"radio\" name=\"modify_type\" value=\"create\" onChange=\"ct_div(2,-1)\" />Create a new data type <br/>".
			"<input type=\"radio\" name=\"modify_type\" value=\"upgrade\" onChange=\"ct_div(2,$val)\" checked/> select an existing data type";
			$ns="document.getElementById('$id').innerHTML='$content';".
				"ct_div(2,$val);";
		}
	}else if ($sn=='2'){
		if($val=='-1'){
			$content=ct_create_content();
			$ns="document.getElementById('$id').innerHTML='$content';";
		}else{
			$content=ct_select_content($val);
			$ns="document.getElementById('$id').innerHTML='$content';";
			if($val!='-2'){
				$ns=$ns."ct_div(2.1,$val);ct_div(2.2,$val);";
			}
		}
		
	}else if ($sn=='2.1'){
		$content=ct_select_sub1_content($val);
		$ns="document.getElementById('$id').innerHTML='$content';";
	}else if ($sn=='2.2'){
		$content=ct_select_sub2_content($val);
		$ns="document.getElementById('$id').innerHTML='$content';";
	}
	return $ns;
}

function ct_create_content(){
	//$db = getDAEDatabase();
	$content="<form ><b>Name: </b><br/><input type=\"text\" id=\"ctc_name\"/><br/>".
		"<b>Description: </b><br/><input type=\"text\" id=\"ctc_des\"/><br/>".
		"<b>Data Item Type: </b><br/><select id=\"ctc_type\">".
		"<option value=\"\"></option>".
		"<option value=\"argument\">argument</option>".
		"<option value=\"files\">files</option>".
		"<option value=\"page_element\">page_element</option>".
		"<option value=\"page_element_property_value\">page_element_property_value</option>".
		"<option value=\"page_image\">page_image</option>".
		"</select><br/>".
		"<input type=\"button\" value=\"save\" onclick=\"ct_save()\"/></form>".
		"<b>NOTE: </b>you need first click the save button then the OK button to choose your new data type.";
	return $content;
}

function ct_select_content($val){
	$db = getDAEDatabase();
	$content="<select id=\"ct_dtid\" onchange=\"ct_text_val()\">";
	$sql="select distinct DATA_ITEM_TYPE from datatype order by DATA_ITEM_TYPE";
	$result=$db->q($sql,array());
	$ditypes=array();
	while($row = $db->fetch_array($result)){
		$tp=$row['DATA_ITEM_TYPE'];
		array_push($ditypes,$tp);
	}
	$content=$content."<option value=\"\"></option>";
	foreach($ditypes as $tp){
		$content=$content."<optgroup label=\"$tp\">";
		$sql="select ID,NAME from datatype where DATA_ITEM_TYPE=:tp order by NAME";
		$variables[':tp']=$tp;
		$result=$db->q($sql,$variables);
		while($row = $db->fetch_array($result)){
			$cstr="";
			if($val==$row['ID']){
				$cstr="selected";
			}
			$content=$content."<option value=\"".$row['ID']."\" $cstr>".$row['NAME']."</option>";
		}
		$content=$content."</optgroup>";
	}
	$content=$content."</select>";
	$content=$content."<b>Description: </b><div id=\"ct_div2.1\"></div><b>Data Item Type: </b><div id=\"ct_div2.2\" ></div>";
	return $content;
}

function ct_select_sub1_content($val){
	$content="";
	$db = getDAEDatabase();
	if(!empty($val)){
		settype($val, 'integer');
		$sql="select DESCRIPTION from datatype where ID=$val";
		$content=$db->r($sql,array());
	}
	return $content;
}

function ct_select_sub2_content($val){
	$content="";
	$db = getDAEDatabase();
	if(!empty($val)){
		settype($val, 'integer');
		$sql="select DATA_ITEM_TYPE from datatype where ID=$val";
		$content=$db->r($sql,array());
	}
	return $content;
}

function save_type(){
	$name=$_POST["name"];
	$des=$_POST["des"];
	$type=$_POST["type"];
	$db = getDAEDatabase();
	$sql="select max(ID)+1 from datatype";
	$id=$db->r($sql,array());
	$sql="insert into datatype(ID,NAME,DESCRIPTION,DATA_ITEM_TYPE) values($id,:name,:des,:type)";
	$variables[':name']=$name;
	$variables[':des']=$des;
	$variables[':type']=$type;
	$db->q($sql,$variables);
	echo $id;
}

function check_version_page($aname,$aversion){
	$db = getDAEDatabase();
	$variables[':aname'] = $aname;
	$sql="select distinct VERSION from algorithm where name=:aname";
	$vlist=array();
	$flag=false;
	$result=$db->q($sql,$variables);
	while($row = $db->fetch_array($result)){
		$tmp=$row['VERSION'];
		array_push($vlist,$tmp);
		if($tmp==$aversion){
			$flag=true;
		}
	}
	if($flag){
		return $vlist;
	}else{
		return array();
	}
}

function submit_page(){
	$vlist=check_version_page($_POST["alg_name"],$_POST["alg_version"]);
	if(!empty($vlist)){
		$str="<script>function check_version(){ var blist=[\"".implode('","',$vlist)."\"]; ".
			"var val=document.getElementById('alg_version').value; ".
			"if(val.trim().length==0){return false;} ".
			"for(i=0;i<=blist.length-1;i++){".
			"if(blist[i]==val){".
			"document.getElementById('blist'+i).style.color='red';".
			"return false;} } return true;}</script>".
			"<h2>please input another version</h2><p>The following version(s) have been found in the database:</p><p>";
		for($i=0;$i<=count($vlist)-1;$i++){
			$str=$str."<span id=\"blist$i\">".$vlist[$i]."</span>";
			if($i<count($vlist)-1){
				$str=$str.", ";
			}
		}
		$str=$str."</p><form action=\"./?q=algorithm/submit_modification/\" method=\"POST\" onsubmit=\"return check_version()\" enctype=\"multipart/form-data\">";
		foreach(array_keys($_POST) as $key){
			if($key=="alg_version"){
				$str=$str."Enter a new version: <input type=\"text\" id=\"$key\" name=\"$key\"/>";
			}else{
				$value=$_POST[$key];
				$str=$str."<input type=\"hidden\" name=\"$key\" value=\"$value\"/>";
			}
		}
		$str=$str."<input type=\"submit\" value=\"resubmit\"></form>";
		return $str;
	}else{
		global $user;
		$uid=$user->uid;
		$pid=getPID($uid);
		$person=new Person($pid);
		$email=$person->email;
	
		$upload_dir="/dae/upload/".$uid."/algorithms/".time().$_FILES["alg_file"]["name"].'/';
		$mode = 0777;
		mkdir($upload_dir, $mode, true);
		$path=$upload_dir.$_FILES["alg_file"]["name"];
		$xmlFile = $path."_metadata.xml";
		$phpFile = $upload_dir . 'setup_' . escapeshellarg(str_replace(' ', '_', $_POST["alg_name"])) . '.php';
		if ($_FILES["alg_file"]["error"] > 0){
  			return "Error: " . $_FILES["alg_file"]["error"] . "<br />";
		}else{
			if (! move_uploaded_file($_FILES["alg_file"]["tmp_name"],$path)) {
			    return "Error: Cannot move file <br/>";
			}
		}
	
		$doc = new DOMDocument();
  		$doc->formatOutput = true;
  		$r=$doc->createElement("metadata");
  		
  		$phpString = "\$inputT = array();\n";
  	
  		$t3=$_POST['total_3'];
		for($i=1;$i<=$t3;$i++){
			$data=$doc->createElement("data");
  			
  			$t=$doc->createElement("id");
			$t->appendChild($doc->createTextNode("Input$i"));
			$data->appendChild($t);
			
			$t=$doc->createElement("name");
			$t->appendChild($doc->createTextNode($_POST["name_3_$i"]));
			$data->appendChild($t);
			
			$cleanName = str_replace(' ', '_', $_POST["name_3_$i"]);
			$phpString .= "\$inputT['$cleanName'] = array('name'=>'$cleanName', 'type'=>'xsd:string');\n";
			
			$t=$doc->createElement("description");
			$t->appendChild($doc->createTextNode($_POST["des_3_$i"]));
			$data->appendChild($t);
			
			$t=$doc->createElement("ordering");
			$t->appendChild($doc->createTextNode($i));
			$data->appendChild($t);
		
			$t=$doc->createElement("required");
			$t->appendChild($doc->createTextNode($_POST["req_3_$i"]));
			$data->appendChild($t);
  			
  			$t=$doc->createElement("type_of");
			$t->appendChild($doc->createTextNode($_POST["dtid_3_$i"]));
			$data->appendChild($t);
		
			$r->appendChild($data);
		}

		$phpString .= "\n\$outputT = array();\n";
		
  		$t4=$_POST['total_4'];
  		for($i=1;$i<=$t4;$i++){
			$data=$doc->createElement("data");
  		
  			$t=$doc->createElement("id");
			$t->appendChild($doc->createTextNode("Output$i"));
			$data->appendChild($t);
		
			$t=$doc->createElement("name");
			$t->appendChild($doc->createTextNode($_POST["name_4_$i"]));
			$data->appendChild($t);
		
			$cleanName = str_replace(' ', '_', $_POST["name_4_$i"]);
			$phpString .= "\$outputT['$cleanName'] = array('name'=>'$cleanName', 'type'=>'xsd:string');\n";
						
			$t=$doc->createElement("description");
			$t->appendChild($doc->createTextNode($_POST["des_4_$i"]));
			$data->appendChild($t);
		
			$t=$doc->createElement("ordering");
			$t->appendChild($doc->createTextNode($i));
			$data->appendChild($t);
		
			$t=$doc->createElement("required");
			$t->appendChild($doc->createTextNode($_POST["req_4_$i"]));
			$data->appendChild($t);
  		
  			$t=$doc->createElement("type_of");
			$t->appendChild($doc->createTextNode($_POST["dtid_4_$i"]));
			$data->appendChild($t);
	
  			$r->appendChild($data);
		}
	
		$alg=$doc->createElement("algorithm");
	
		$t=$doc->createElement("name");
		$t->appendChild($doc->createTextNode($_POST["alg_name"]));
		$alg->appendChild($t);
	
		$t=$doc->createElement("version");
		$t->appendChild($doc->createTextNode($_POST["alg_version"]));
		$alg->appendChild($t);
	
		$t=$doc->createElement("path");
		$t->appendChild($doc->createTextNode($path));
		$alg->appendChild($t);
	
		$t=$doc->createElement("description");
		$t->appendChild($doc->createTextNode($_POST["alg_description"]));
		$alg->appendChild($t);
	
		$t=$doc->createElement("time_complexity");
		$t->appendChild($doc->createTextNode($_POST["alg_timecomplexity"]));
		$alg->appendChild($t);
	
		$t=$doc->createElement("space_complexity");
		$t->appendChild($doc->createTextNode($_POST["alg_spacecomplexity"]));
		$alg->appendChild($t);
	
		for($i=1;$i<=$t3;$i++){
			$t=$doc->createElement("algorithm_input");
			$t2=$doc->createElement("data_id");
			$t2->appendChild($doc->createTextNode("Input$i"));
			$t2->setAttribute("type","internal");
			$t->appendChild($t2);
			$alg->appendChild($t);
		}
		for($i=1;$i<=$t4;$i++){
			$t=$doc->createElement("algorithm_output");
			$t2=$doc->createElement("data_id");
			$t2->appendChild($doc->createTextNode("Output$i"));
			$t2->setAttribute("type","internal");
			$t->appendChild($t2);
			$alg->appendChild($t);
		}
		$t=$doc->createElement("defines_algorithm");
		$t2=$doc->createElement("person_id");
		$t2->appendChild($doc->createTextNode($email));
		$t->appendChild($t2);
		$alg->appendChild($t);
	
		$r->appendChild($alg);
	
		$doc->appendChild($r);
	
		$str=$doc->saveXML();
	
	
	
		$fh = fopen($xmlFile, 'w') or die("can't open file $xmlFile");
		fwrite($fh, $str);
		fclose($fh);
		
		$fh = fopen($phpFile, 'w') or die("can't open file $phpFile");
		fwrite($fh, $phpString);
		fclose($fh);

		$link="";
        $params['account'] = user_load(1);
        
        $params['content'] = $str;
        $params['path'] = $upload_dir;
        
        drupal_mail('algorithms', 'modify', user_load(1)->mail, user_preferred_language($account), $params);
		return "<h2>Successfully Submitted!</h2>
			<p>
				A message has been sent to the administrator. You should be contacted shortly. 
				If not, please contact <a href=\"mailto:" .user_load(1)->mail. "\">the DAE crew</a>
			</p>
			<p>The metadata of your algorithm is <p><pre>". htmlentities($str) . "</pre></p></p>.";
	}
}


function algorithms_mail($key, &$message, $params) {
    global $user;
    
    $data['user'] = $params['account'];
    $variables = user_mail_tokens($user, $message['language']);
    switch($key) {
      case 'modify':
        $message['subject'] = t('Notification from !site', $variables);
        $message['body'][] = t("!username (!mailto) has uploaded an algorithm description to" . $params['path'] . "\n\n" . $params['content'], $variables);
        break;
      default:
        $message['subject'] = t('Unknown notification from !site', $variables);
        $message['body'][] = t("!username (!mailto) has provoked an unhandled event in the 'algorithms' module", $variables);
        break;  
    }
}

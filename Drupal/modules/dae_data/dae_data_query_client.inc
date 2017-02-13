<?php
function query_client(){
	global $base_url;
    global $user;
    
    $limit=$_REQUEST['limit'];
    $pitype_str=$_REQUEST['pitype'];
    $dataset_str=$_REQUEST['dataset'];
    $petype_str=$_REQUEST['petype'];
    $pedtprop_str=$_REQUEST['pedtprop'];
    $peval_str=$_REQUEST['peval'];
    
	$fpitype=$_REQUEST['lpitype'];
    $fdataset=$_REQUEST['ldataset'];
    $fpetype=$_REQUEST['lpetype'];
    $fpedtprop=$_REQUEST['lpedtprop'];
    
	$pitype_arr=idstr2arr($pitype_str);
	$dataset_arr=idstr2arr($dataset_str);
	$petype_arr=idstr2arr($petype_str);
	$pedtprop_arr=idstr2arr($pedtprop_str);
	$peval_arr=valstr2arr($peval_str);

	$wsdl = "$base_url/services/soap?wsdl";
	$client = new nusoap_client($wsdl, false);
	
    $err = $client->getError();
    if ($err) {
       drupal_set_message(print_r($err,true),error);
    }
    
	DEFINE('USE_SESSION', FALSE);
	$client->debug_flag=false;
	

	$result = $client->call('DAE Data.PageImage Query', 
		array('limit'=>$limit,
			'hasPageImageType'=>$pitype_arr,
			'inDataset'=>$dataset_arr,
			'hasPageElementType'=>$petype_arr,
			'hasPageElementDataTypeProperty'=>$pedtprop_arr,
			'hasPageElementPropertyValue'=>$peval_arr,
			'logicType_hasPageImageType'=>$fpitype,
			'logicType_inDataset'=>$fdataset,
			'logicType_hasPageElementType'=>$fpetype,
			'logicType_hasPageElementDataTypeProperty'=>$fpedtprop,
			)
		);
	
    if ($client->fault) {
        drupal_set_message(print_r($result,true),error);
    } else {
        $err = $client->getError();
        if ($err) {
           drupal_set_message(print_r($err,true),error);
        }
    }
	
	$orig=$client->responseData;
	
	$after=ereg_replace("<\\?xml[\x20-\x7E]+\\?>","\\0 <?xml-stylesheet type=\"text/xsl\" href=\"$base_url/page_image_query.xsl\"?>",$orig);
	drupal_set_header('Content-Type: text/xml; utf-8');
	print $after;
}

function query_preview(){
    $limit=$_REQUEST['limit'];
    $pitype_str=$_REQUEST['pitype'];
    $dataset_str=$_REQUEST['dataset'];
    $petype_str=$_REQUEST['petype'];
    $pedtprop_str=$_REQUEST['pedtprop'];
    $peval_str=$_REQUEST['peval'];
    
    $fpitype=$_REQUEST['lpitype'];
    $fdataset=$_REQUEST['ldataset'];
    $fpetype=$_REQUEST['lpetype'];
    $fpedtprop=$_REQUEST['lpedtprop'];
    
	$pitype_arr=idstr2arr($pitype_str);
	$dataset_arr=idstr2arr($dataset_str);
	$petype_arr=idstr2arr($petype_str);
	$pedtprop_arr=idstr2arr($pedtprop_str);
	$peval_arr=valstr2arr($peval_str);
	
	$sql=genSQL($limit,$pitype_arr,$dataset_arr,$petype_arr,$pedtprop_arr,$peval_arr,$fpitype,$fdataset,$fpetype,$fpedtprop);
	echo $sql;
}


function my_set_int_type($arr){
	$narr=array();
	foreach($arr as $ele){
		$nele=$ele;
		settype($nele, 'integer');
		array_push($narr,$nele);
	}
	return $narr;
}

function genSQL($lim,$opitList,$odsList,$opetList,$opepList,$opevList,$fpit,$fds,$fpet,$fpep){

    
	settype($lim, 'integer');
	$pitList=my_set_int_type($opitList);
	$dsList=my_set_int_type($odsList);
	$petList=my_set_int_type($opetList);
	$pepList=my_set_int_type($opepList);
	$pevList=$opevList;
	
	$sql='';
	if($lim>0){
		$sql="select * from (";
	}
	
	$sql=$sql."select distinct(pi.ID) from page_image pi ";
	if(count($pitList)>0){
		if($fpit){
	        $sql=$sql."inner ";
		}else{
		    $sql=$sql."left ";
		} 
		$sql=$sql."join associate_datatype_data_item addiA on addiA.DATA_ITEM_ID = pi.ID ";
	}
	if(count($dsList)>0){
		if($fds){
	        $sql=$sql."inner ";
		}else{
		    $sql=$sql."left ";
		} 
	    $sql=$sql."join includes_page_image ipi on ipi.PAGE_IMAGE_ID = pi.ID ";
	}
	if(count($petList)+count($pepList)+count($pevList)>0){
		$sql=$sql."left join contains_page_element cpe on pi.ID = cpe.PAGE_IMAGE_ID ";
	}
	if(count($petList)>0){
	    if($fpet){
	        $sql=$sql."inner ";
		}else{
		    $sql=$sql."left ";
		} 
		$sql=$sql."join associate_datatype_data_item addiB on addiB.DATA_ITEM_ID = cpe.PAGE_ELEMENT_ID ";
	}
	if(count($pepList)+count($pevList)>0){
	    if($fpep){
	        $sql=$sql."inner ";
		}else{
		    $sql=$sql."left ";
		} 
		$sql=$sql."join has_value hv on cpe.PAGE_ELEMENT_ID = hv.PAGE_ELEMENT_ID ";
	}
	if(count($pepList)>0){
	    if($fpep){
	        $sql=$sql."inner ";
		}else{
		    $sql=$sql."left ";
		} 
		$sql=$sql."join value_of vo on vo.PAGE_ELEMENT_PROPERTY_VALUE_ID = hv.PAGE_ELEMENT_PROPERTY_VALUE_ID ";
	}

	if(count($pevList)>0){
		$sql=$sql."inner join page_element_property_value pepv on pepv.ID = hv.PAGE_ELEMENT_PROPERTY_VALUE_ID ";
	}

	// where clause
	$bfollow=false;
	if(count($pitList)+count($dsList)+count($petList)+count($pepList)+count($pevList)>0){
		$sql=$sql."where ";
	}
	if(count($pitList)>0){
		if($bfollow){
			$sql=$sql."and ";
		}
		$tmp=implode(",",$pitList);
		$sql=$sql."(addiA.DATATYPE_ID ";
		if($fpit==0){
			$sql=$sql."not ";
		}
		$sql=$sql."in ($tmp) ";
		if($fpit==0){
		    $sql=$sql."or addiA.DATATYPE_ID is null";
		}
		$sql=$sql.")";
		$bfollow=true;
	}
	if(count($dsList)>0){
		if($bfollow){
			$sql=$sql."and ";
		}
		$tmp=implode(",",$dsList);
		$sql=$sql."(ipi.DATASET_ID ";
		if($fds==0){
			$sql=$sql."not ";
		}
		$sql=$sql."in (select distinct(associating_dataset_id) from associate_dataset ".
			"start with DATASET_ID in ($tmp) ".
			"connect by prior ASSOCIATING_DATASET_ID=DATASET_ID) or ipi.DATASET_ID in ($tmp) )";
		$bfollow=true;
	}
	if(count($petList)>0){
		if($bfollow){
			$sql=$sql."and ";
		}
		$tmp=implode(",",$petList);
		$sql=$sql."(addiB.DATATYPE_ID ";
		if($fpet==0){
			$sql=$sql."not ";
		}
		$sql=$sql."in ($tmp) ";
	    if($fpet==0){
			$sql=$sql."or addiB.DATATYPE_ID is null";
		}
		$sql=$sql.")";
		$bfollow=true;
	}
	if(count($pepList)>0){
		if($bfollow){
			$sql=$sql."and ";
		}
		$tmp=implode(",",$pepList);
		$sql=$sql."(vo.DATA_TYPE_PROPERTY_ID ";
		if($fpep==0){
			$sql=$sql."not ";
		}
		$sql=$sql."in ($tmp) ";
		if($fpep==0){
			$sql=$sql."or vo.DATA_TYPE_PROPERTY_ID is null";
		}
		$sql=$sql.")";
		$bfollow=true;
	}
	if(count($pevList)>0){
		if($bfollow){
			$sql=$sql."and ";
		}
		$sql=$sql."(";
		$bev=false;
		foreach($pevList as $pev){
			if($bev){
				$sql=$sql." or ";
			}
			$sql=$sql."pepv.VALUE like '%$pev%'";
			$bev=true;
		}
		$sql=$sql." escape '\')";
		$bfollow=true;
	}
	
	if($lim>0){
		$sql=$sql." order by dbms_random.value()) where ROWNUM<=$lim";
	}
	
	return $sql;
}

function idstr2arr($idstr){
	$arr=array();
	if($idstr!="[]"){
		$nstr=substr($idstr,1,strlen($idstr)-2);
		$nstr=trim($nstr);
		if(strlen($nstr)>0){
			$arr=explode(",",$nstr);	 
		}
	}
	return $arr;
}

function valstr2arr($valstr){
	$vals=array();
	$str=trim($valstr);
	if(strlen($str)>0){
		$str=substr($str,1,strlen($str)-2);
		$vsp="','";
		$parts=explode($vsp,$str);;
		for($i=0;$i<=count($parts)-1;$i++){
			$curval=my_codec($parts[$i]);
			array_push($vals,$curval);
		}
	}
	return $vals;
}

function my_codec($str){
	$nstr=urldecode($str);
	$nstr=str_replace("\\","\\\\",$nstr);
	$nstr=str_replace("'","''",$nstr);
	$nstr=str_replace("%","\\%",$nstr);
	$nstr=str_replace("_","\\_",$nstr);
	return $nstr;
}
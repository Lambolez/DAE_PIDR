<?php

require_once("./".drupal_get_path('module','daedatabase')."/daedatabase_db.inc");

function dataitem_service_descript($idList) {
    
    $FILE_PATH=url("browse/dataitem/download/",array('absolute' => true));
	$rslt2=array();
	$db = getDAEDatabase();
	foreach($idList as $id){
	    $variables[':id'] = $id;
		$sql='select DESCRIPTION,FLAG from data_item where ID=:id';
		$result=$db->q($sql,$variables);
		if(count($result)==1){
			$row = $db->fetch_array($result);
			$description=$row['DESCRIPTION'];
			//$copyright=$row['COPYRIGHT'];
			$flag=$row['FLAG'];
			$copyright_list=array();
			$sql='select FILE_ID from has_copyright where DATA_ITEM_ID=:id';
			$rs1=$db->q($sql,$variables);
			while($row = $db->fetch_array($rs1)){
				array_push($copyright_list,$row['FILE_ID']);
			}
			switch($flag){
			case 'files': //file
				$name=$db->r('select NAME from files where id=:id',$variables);
				$path=$FILE_PATH.$id;			
				$item=array('file_info'=>
						array('id'=>$id,
							'name'=>$name,
							'description'=>$description,
							'copyright_list'=>$copyright_list,
							'path'=>$path,
						)
					);
				break;
			case 'dataset': //dataset
				$name=$db->r('select NAME from dataset where id=:id',$variables);
				$type_list=array();
				$sql='select DATATYPE_ID from associate_datatype_data_item where DATA_ITEM_ID=:id';
				$rs2=$db->q($sql,$variables);
				while($row = $db->fetch_array($rs2)){
					array_push($type_list,$row['DATATYPE_ID']);
				}
				$file_list=array();
				$sql='select FILE_ID from includes_file where DATASET_ID=:id';
				$rs2=$db->q($sql,$variables);
				while($row = $db->fetch_array($rs2)){
					array_push($file_list,$row['FILE_ID']);
				}
				$page_element_list=array();
				$sql='select PAGE_ELEMENT_ID from includes_page_element where DATASET_ID=:id';
				$rs2=$db->q($sql,$variables);
				while($row = $db->fetch_array($rs2)){
					array_push($page_element_list,$row['PAGE_ELEMENT_ID']);
				}
				$page_image_list=array();
				$sql='select PAGE_IMAGE_ID from includes_page_image where DATASET_ID=:id';
				$rs2=$db->q($sql,$variables);
				while($row = $db->fetch_array($rs2)){
					array_push($page_image_list,$row['PAGE_IMAGE_ID']);
				}
				$dataset_list=array();
				$sql='select ASSOCIATING_DATASET_ID from associate_dataset where DATASET_ID=:id';
				$rs2=$db->q($sql,$variables);
				while($row = $db->fetch_array($rs2)){
					array_push($dataset_list,$row['ASSOCIATING_DATASET_ID']);
				}
				$property_value_list=array();
				$sql='select PAGE_ELEMENT_PROPERTY_VALUE_ID from includes_pe_pv where DATASET_ID=:id';
				$rs2=$db->q($sql,$variables);
				while($row = $db->fetch_array($rs2)){
					array_push($property_value_list,$row['PAGE_ELEMENT_PROPERTY_VALUE_ID']);
				}
				$item=array('dataset_info'=>
							array('id'=>$id,
								'name'=>$name,
								'description'=>$description,
								'copyright_list'=>$copyright_list,
								'type_list'=>$type_list,
								'file_list' => $file_list,
								'page_element_list' => $page_element_list,
								'page_image_list' => $page_image_list,
								'dataset_list' => $dataset_list,
								'property_value_list' =>$property_value_list,
							)
						);
				break;
			case 'page_element': //page element
				$type_list=array();
				$sql='select DATATYPE_ID from associate_datatype_data_item where DATA_ITEM_ID=:id';
				$rs2=$db->q($sql,$variables);
				while($row = $db->fetch_array($rs2)){
					array_push($type_list,$row['DATATYPE_ID']);
				}
				$value_list=array();
				$sql='select PAGE_ELEMENT_PROPERTY_VALUE_ID from has_value where PAGE_ELEMENT_ID=:id';
				$rs2=$db->q($sql,$variables);
				while($row = $db->fetch_array($rs2)){
					array_push($value_list,$row['PAGE_ELEMENT_PROPERTY_VALUE_ID']);
				}
				
				$in_PI=$db->r('select PAGE_IMAGE_ID from contains_page_element where PAGE_ELEMENT_ID=:id',$variables);
				
				$item=array('page_element_info'=>
						array('id'=>$id,
							'in_page_image'=>$in_PI,
							'description'=>$description,
							'copyright_list'=>$copyright_list,
							'type_list'=>$type_list,
							'property_value_list'=>$value_list,
						)
					);
				break;
			case 'page_image': //page image
				$sql='select VDPI,HDPI,SKEW,WIDTH,HEIGHT from page_image where ID=:id';
				$rs2=$db->q($sql,$variables);
				$row=$db->fetch_array($rs2);
				$vdpi=$row['VDPI'];
				$hdpi=$row['HDPI'];
				$skew=$row['SKEW'];
				$path=$FILE_PATH.$id;
				$width=$row['WIDTH'];
				$height=$row['HEIGHT'];
				
				$type_list=array();
				$sql='select DATATYPE_ID from associate_datatype_data_item where DATA_ITEM_ID=:id';
				$rs2=$db->q($sql,$variables);
				while($row = $db->fetch_array($rs2)){
					array_push($type_list,$row['DATATYPE_ID']);
				}
				
				$page_element_list=array();
				$sql='select PAGE_ELEMENT_ID from contains_page_element where PAGE_IMAGE_ID=:id';
				$rs2=$db->q($sql,$variables);
				while($row = $db->fetch_array($rs2)){
					array_push($page_element_list,$row['PAGE_ELEMENT_ID']);
				}
				
				$item=array('page_image_info'=>
						array('id'=>$id,
							'description'=>$description,
							'copyright_list'=>$copyright_list,
							'type_list'=>$type_list,
							'page_element_list'=>$page_element_list,
							'vdpi'=>$vdpi,
							'hdpi'=>$hdpi,
							'skew'=>$skew,
							'path'=>$path,
							'width'=>$width,
							'height'=>$height,
						)
					);
				break;
			case 'page_element_property_value': //property value
				$in_PE=$db->r('select PAGE_ELEMENT_ID from has_value where PAGE_ELEMENT_PROPERTY_VALUE_ID=:id',$variables);
				$property=$db->r('select DATA_TYPE_PROPERTY_ID from value_of where PAGE_ELEMENT_PROPERTY_VALUE_ID=:id',$variables);
				$value=$db->r('select VALUE from PAGE_ELEMENT_PROPERTY_VALUE where ID=:id',$variables);
				$item=array('property_value_info'=>
						array('id'=>$id,
							'description'=>$description,
							'copyright_list'=>$copyright_list,
							'in_page_element'=>$in_PE,
							'property'=>$property,
							'value'=>(is_null($value)) ? '' : $value->load(),
						)
					);
				break;
			default:
				$item=array('unknown_data_item'=>
						array('id'=>$id,
							'description'=>'The type of this data item is unknown to the system',
						)
					);
				break;
			}
			if(!empty($item)){
				array_push($rslt2,$item);
				unset($item);
			}
		}else{
			$item=array('unknown_data_item'=>
					array('id'=>$id,
						'description'=>'This data item does not exist in the system',
					)
				);
			array_push($rslt2,$item);
		}
	}
	
	return $rslt2;
}


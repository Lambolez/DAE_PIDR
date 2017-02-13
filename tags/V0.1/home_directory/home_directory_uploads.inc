<?php
// $Id:$

/**
 * @file
 * Provides the my uploads page for home directory.
 */
  
/**
 * My Uploads Page
 */ 
function home_directory_uploads()
{
	global $uploads, $user;
	
	$url     = 'uploads/view';
	
	$loc     = hd_displayMessage();
	
	$output  = hd_getBreadcrumbs($loc, $url, "Uploads", $dir);
	
	if(file_exists($uploads . $dir))
	{
		$alt = 0;
		
		$files   = scandir($uploads.$dir);
	
		$moves = hd_getCurrentDir($dir,$url,$files);
	
		$output .= hd_getDirs($uploads,$dir,$url,$files,$alt);
	
		$output .= hd_getFiles($uploads,$dir,$files,$moves,$alt);
	}
	else
	{
		drupal_set_message(t('Directory %d does not exist.',array('%d' => "uploads" . $dir)),'warning');
	}
	
	$amount_used_result = db_query("select used from {user_storage} where uid = " . $user->uid);
	
	$row = db_fetch_array($amount_used_result);
		
	$used = ($row['used'] / variable_get('home_directory_size', 100000));
	
	$use = $used;
	
	if($used > 1)
		$use = 1; 
	
	$usage = hd_getUsageString($row['used']);
	
	$output .= "<div id=\"hd-usage\"><div id=\"hd-usage-container\" ><div class=\"hd-usage-container-right\"><div style=\"width:" . sprintf("%3.0f",$use*293) . "px\" id=\"hd-used\">&nbsp;</div></div></div>".sprintf("%1.0f",$used*100)."% ($usage) </div>";

	
	return $output;
	
}
function hd_getUsageString($used)
{
	$total = variable_get('home_directory_size', 1000000);
	
	$used = hd_sizeString($used);
	$total = hd_sizeString($total);
	
	return $used . " / " . $total;
		
}
function hd_sizeString($bytes)
{
	$gb = 1073741824;
	$mb = 1048576;
	$kb = 1024;

	if($bytes >= $gb)
		$s = sprintf("%.2f",$bytes/$gb) . " GB";
	else if($bytes >= $mb)
		$s = sprintf("%.2f",$bytes/$mb) . " MB";
	else
		$s = sprintf("%.2f",$bytes/$kb) . " KB";
	
	return $s;
}
/**
 * displayMessage 
 */ 
function hd_displayMessage()
{
	$loc = arg();
	
	unset($loc[0]);
	
	switch($loc[1])
	{
		case 'uploaded';
			drupal_set_message(t('%file uploaded successfully.',array('%file' => end($loc))),'status');
			end($loc);
			unset($loc[key($loc)]);
			break;
		case 'deleted':
			drupal_set_message(t('%file deleted successfully.',array('%file' => end($loc))),'status');
			end($loc);
			unset($loc[key($loc)]);
			break;
		case 'notempty':
			drupal_set_message(t('%file is not empty.',array('%file' => end($loc))),'warning');
			end($loc);
			unset($loc[key($loc)]);
			break;
		case 'moved':
			drupal_set_message(t('%file was moved successfully.',array('%file' => end($loc))),'status');
			end($loc);
			unset($loc[key($loc)]);
			break;
	}
	
	unset($loc[1]);
	
	return $loc;
}

/**
 * getBreadcrumbs 
 */ 
function hd_getBreadcrumbs($loc, $url, $root, &$dir)
{
	
	$directoryImage = "<img src=\"".drupal_get_path("module","home_directory")."/images/folder.png\" alt=\"\" />";
	
	//$breadcrumbs = "<div id=\"hd-usage\"><div style=\"width:" . sprintf("%3.0f",$used*250) . "px\" id=\"hd-used\">". sprintf("%1.0f",$used*100) ."%</div></div>";
	
	$breadcrumbs .= "<div class=\"hd-breadcrumbs\"><span class=\"left\">".$directoryImage.l($root,"uploads");
	
	foreach($loc as $folder):
		
		$dir.="/".$folder;
		$breadcrumbs.=" / ".$directoryImage.l($folder,$url.$dir);
	
	endforeach;
	
	//popups_add_popups();
	
	$breadcrumbs .= "</span>
	<img src=\"".drupal_get_path("module","home_directory")."/images/upload.png\" alt=\"\" />".l("Upload File","upload".$dir,array('attributes' => array('class' => 'popups-form') )).
	"&nbsp;&nbsp;<img src=\"".drupal_get_path("module","home_directory")."/images/add.png\" alt=\"\" />"/*<a href='#' onClick='newSubDirectory()'>Make Subdirectory</a>"; //*/.l("Make Subdirectory","upload/mkdir".$dir,array('attributes' => array('class' => 'popups-form') ));
	
	//$breadcrumbs .= l("Test Lightbox","upload",array('attributes' => array('rel' => 'lightframe[|width:400px; height:300px; scrolling: auto;]') ));
	
	$breadcrumbs .= "</div>";
	
	return $breadcrumbs;
}

/**
 * getDirectories
 */ 
function hd_getDirs($from,$dir,$url,&$files,&$alt)
{	
	
	unset($files[0]);
	unset($files[1]);

	$output = "<div id=\"directories\">";
	
	foreach($files as $f => $file):
	
		if(!is_file($from.$dir.'/'.$file))
		{
			$output .= "<div class=\"hd-upload_row " . ($alt%2==0?"gray":""); 
			
			$output .= "\">\n<span class=\"left\"><img src=\"".drupal_get_path("module","home_directory")."/images/folder.png\" width=\"24\" height=\"24\" alt=\"\" />";
			
			$output .= l($file,$url.$dir."/".$file)."\n</span>";
			
			$output .= l("<img class=\"delete\" src=\"".drupal_get_path("module","home_directory")."/images/cancel.png\" alt=\"\" /> ","upload/rmdir$dir/$file", array('html' => true));
			
			$output .= "</div>\n";
			
			unset($files[$f]);
			
			$alt++;
		}	
		
	
	endforeach;
	
	$output .= "</div>";
	
	return $output;

}
function hd_getCurrentDir($dir,$url,$files)
{
	global $uploads;

	$i = 0;
	
	$dirs = array();
	
	unset($files[0]);
	unset($files[1]);
	
	foreach($files as $file):
	
		if(is_dir($uploads.$dir.'/'.$file))
		{
			$dirs[$i] = $dir.'/'.$file;
			$i++;
		}	
	
	endforeach;
	
	return $dirs; 
}
/**
 * getFiles
 */ 
function hd_getFiles($from,$dir,$files,$moves,$alt)
{
	global $uploads;

	foreach($files as $file):
		
		$output.= "<div class=\"hd-upload_row " . ($alt%2==0?"gray":""); 
		
		$output.= "\">\n<div class=\"hd-upload_filename\"><img src=\"?q=getThumb$dir/$file\" width=\"24\" height=\"24\" />\n";
		
		$output.= l($file,"getFile/uploads$dir/$file");
		
		$output.= "</div><span class=\"hd-upload_filesize\">" . hd_sizeString(filesize($uploads . '/' . $dir . '/' . $file)) . "</span>";
		
		$output.= "<form class=\"hd-mv\" action=\"#\"><select onChange=\"moveFile(this);\"><option value=\"\">Move</option>";
		
		$up = explode("/",$dir);
		
		unset($up[0]);
		
		end($up);
		unset($up[key($up)]);
		
		$udir = implode("/",$up);
		
		if(empty($udir))
			$output.="<option value=\"?q=upload/move$dir/$file/to/$file\" >Move Parent Directory</option>";
		else if(!empty($udir))
			$output.="<option value=\"?q=upload/move$dir/$file/to/$udir/$file\" >Move Parent Directory</option>";
		
		
		foreach($moves as $d):
		
			$output.="<option value=\"?q=upload/move$dir/$file/to$d/$file\" >Move to $d</option>";
		
		endforeach;		
		
		$output.="</select></form>\n";
	
		$output.= l("<img class=\"delete\" src=\"".drupal_get_path("module","home_directory")."/images/cancel.png\" alt=\"\" /> ","upload/delete$dir/$file", array('html' => true))."</div>";
		
		$alt++;
	
	endforeach;
			
	return $output;
}

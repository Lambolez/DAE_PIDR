<?php
// $Id:$

/**
 * @file
 * This file creates the My Uploads page for a user.
 *
 */
  
/**
 *  Page callback for uploads 
*/
function home_directory_uploads()
{
	$ui = new MyUploadsPage();

	return $ui->build_uploads_page();
}

/**
 *  MyUploadsPage class.  This class is used to put all the pieces of the 
 *  My Uploads page together.  
*/
class MyUploadsPage
{
	///The drupal base URL for viewing uploads so that directory views can be linked to
	var $url;
	///The arguments of the page after uploads/view/
	var $loc;
	///The HTML to be returned to create the page
	var $output;
	///The subdirectory currently being viewed
	var $currentDirectory;
	///Array of files and folders in the directory currently being viewed ($currentDirectory)
	var $files;
	///The base filesystem path to the user's home folder
	var $from;
	///Options where files and folders may be moved
	var $moveTo;
	///The dae_ui instance which renders folder and file rows
	var $ui;
	///The number row in the list of files and folders
	var $row_no;
	
    /**
    *  Gathers all the bits of output to create the uploads page
    */
	function build_uploads_page()
	{
		global $uploads, $user;
	
		require_once("./".drupal_get_path('module','dae_ui')."/dae_ui.module");
		
		$this->ui      = new UploadUI('dae-upload');
		
		$this->url     = 'uploads/view';
		
		$this->from    = $uploads;
		
		$this->displayMessage();
		
		$output        = $this->getBreadcrumbs($loc, $url, "Uploads", $dir);
		
		if(file_exists($this->from . $this->currentDirectory)) //Make sure requested directory exists
		{
			$this->row_no  = 0;
			
			$this->files   = scandir($this->from.$this->currentDirectory); //get files in requested directory
		
			$this->getCurrentDir(); //sets current directory and builds moveTo list
		
			$output .= $this->getOperations(); // 
		
			$output .= $this->getSelectors();
			
			$output .= $this->getBatchForm();
		
			$this->getDirs();
		
			$this->getFiles();
			
			$output .= $this->ui->getOutput();
			
			$output .= "</form>";
		}
		else
		{
			drupal_set_message(t('Directory %d does not exist.',array('%d' => "uploads" . $this->currentDirectory)),'warning');
		}
		
		$output .= $this->getUsage();	
		
		return $output;
		
	}
	/**
 	*  Gets a formmated string expressing the users remaining space
    */
	function getUsageString($used)
	{
		$total = variable_get('home_directory_size', 1000000);
		
		$used = $this->sizeString($used);
		$total = $this->sizeString($total);
		
		return $used . " / " . $total;
			
	}
	/**
 	*  Formats a quantity of bytes into the largest possible measure with two decimal places
    */
	function sizeString($bytes)
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
 	*  Retrieves user's usage amount from the database
    */
	function getUsage()
	{
		global $user;
		
		$amount_used_result = db_query("select used from {user_storage} where uid = " . $user->uid);
		
		$row = db_fetch_array($amount_used_result);
	
		$used = ($row['used'] / variable_get('home_directory_size', 100000));
		
		$use = $used;
		
		if($used > 1) $use = 1; //just in case usage is greater than 100%, bar will only be 100%
		
		$usage = $this->getUsageString($row['used']);
		
		$usageBar = "<div id=\"hd-usage\">
						<div id=\"hd-usage-container\" >
							<div class=\"hd-usage-container-right\">
								<div style=\"width:" . sprintf("%3.0f",$use*293) . "px\" id=\"hd-used\">
									&nbsp;
								</div>
							</div>
						</div>"
						.sprintf("%1.0f",$used*100)
						."% ($usage)
					</div>";
		
		return $usageBar;
	}
	/**
	*  Displays a message above the uploads page if part of the arguments
	*/ 
	function displayMessage()
	{
		$this->loc = arg();

		unset($this->loc[0]); //remove uploads from the drupal arguments
		
		switch($this->loc[1]) //check the 
		{
			case 'uploaded';
				drupal_set_message(t('%file uploaded successfully.',array('%file' => end($this->loc))),'status');
				end($this->loc);
				unset($this->loc[key($this->loc)]);
				break;
			case 'deleted':
				drupal_set_message(t('!file were deleted successfully.',array('!file' => $_SESSION['temp'])),'status');
				end($this->loc);
				unset($this->loc[key($this->loc)]);
				break;
			case 'notempty':
				drupal_set_message(t('%file is not empty.',array('%file' => end($this->loc))),'warning');
				end($this->loc);
				unset($this->loc[key($this->loc)]);
				break;
			case 'moved':
				drupal_set_message(t('!file were moved successfully.',array('!file' => $_SESSION['temp'])),'status');
				end($this->loc);
				unset($this->loc[key($this->loc)]);
				break;
			case 'self-move':
				drupal_set_message(t('Cannot move a directory into itself'),'status');
				end($this->loc);
				unset($this->loc[key($this->loc)]);
				break;
		}
		unset($this->loc[1]);
	}
	
	/**
	 * getBreadcrumbs 
	 */ 
	function getBreadcrumbs()
	{
		$directoryImage = "<img src=\"".drupal_get_path("module","home_directory")."/images/folder.png\" width=\"16\" height=\"16\" alt=\"\" />";
		
		$breadcrumbs .= "<div class=\"hd-breadcrumbs\"><span class=\"left\">".$directoryImage.l("Uploads",$this->url);
			
		foreach($this->loc as $folder):
			
			if($folder != "..")
			{
				$this->currentDirectory.="/".$folder;
				$breadcrumbs.=" / ".$directoryImage.l($folder,$this->url.$this->currentDirectory);
			}
		
		endforeach;
		
		$breadcrumbs .= "</span>".
		
		"<span id=\"hd-upload-file\" class=\"icon-link\">".l("Upload File","upload".$this->currentDirectory)."</span>".
		"<span id=\"hd-make-subdirectory\" class=\"icon-link\">".l("Make Subdirectory","upload/mkdir".$this->currentDirectory)."</span>";
		
		$breadcrumbs .= "</div>";
		
		return $breadcrumbs;
	}
	
	/**
	 * getDirectories
	 */ 
	function getDirs()
	{	

		foreach($this->files as $f => $file):
		
			if(!is_file($this->from.$this->currentDirectory.'/'.$file))
			{				
				$info['left'] = l($file,$this->url.$this->currentDirectory."/".$file);
				$info['click'] = "";
				$info['checkbox']['name'] = "$this->row_no";
				$info['checkbox']['value'] = "$this->currentDirectory/$file";
				$info['checkbox']['class'] = "dirBox";
				$info['hidden'] = array();
				$info['children'] = "data-leaf";
				$info['image'] = "<img src=\"".drupal_get_path("module","home_directory")."/images/folder.png\" width=\"24\" height=\"24\" alt=\"\" />";
				$info['onRowClick'] = "show()";	
		
				$this->ui->addRecord($info);
				$this->row_no++;
				unset($this->files[$f]);		
			}	
		
		endforeach;
		
		return $output;
	
	}
	function getCurrentDir()
	{
	
		$i = 0;
		
		$dirs = array();
		
		foreach($this->files as $key => $file):
		
			if($file == ".." || $file == ".")
				unset($this->files[$key]);
			else
				if(is_dir($this->from.$this->currentDirectory.'/'.$file))
				{
					$this->moveTo[$i] = $file;
					$i++;
				}
		
		endforeach;
		
	}
	/**
	 * getFiles
	 */ 
	function getFiles()
	{
		foreach($this->files as $file):
		
			$ext = home_directory_get_file_extension($file);
			
			$fullPathToFile = $this->from . '/' . $this->currentDirectory . '/' . $file;

			$info['left'] = l($file,"getFile/uploads$this->currentDirectory/$file",array('attributes' => array('onClick' => 'event.stopPropagation();')));
			$info['click'] = " click";
			$info['checkbox']['name'] = "$this->row_no";
			$info['checkbox']['value'] = "$this->currentDirectory/$file";
			$info['checkbox']['class'] = "fileBox";
			$info['children'] = "collap";
			$info['onRowClick'] = "showFileInfo(this)";
			$info['hidden']['file_info']['title'] = "File Information";
			$info['hidden']['file_info']['class'] = "upload_info_basics";
			$info['hidden']['file_info']['label']['class'] = "upload_labels";
			$info['hidden']['file_info']['label'][1] = "Filetype:";
			$info['hidden']['file_info']['label'][2] = "Location:";
			$info['hidden']['file_info']['label'][3] = "Filesize:";
			$info['hidden']['file_info']['label'][4] = "Upload Date:";
			$info['hidden']['file_info']['info']['class'] = "upload_info";
			$info['hidden']['file_info']['info'][1] = $ext;
			$info['hidden']['file_info']['info'][2] = $this->currentDirectory.'/'.$file;
			$info['hidden']['file_info']['info'][3] = $this->sizeString(filesize($fullPathToFile));
			$info['hidden']['file_info']['info'][4] = date ("F d Y H:i:s.", filemtime($fullPathToFile));
			
			if(home_directory_check_image($ext)):

				$imageInfo = getimagesize($fullPathToFile);
				
				$info['hidden']['image_info']['title'] = "Image Information";
				$info['hidden']['image_info']['class'] = "upload_info_image";
				$info['hidden']['image_info']['label']['class'] = "upload_info_image";
 				$info['hidden']['image_info']['label'][1] = "Width:";
				$info['hidden']['image_info']['label'][2] = "Height:";
				$info['hidden']['image_info']['label'][3] = "Color Model:";
				$info['hidden']['image_info']['label'][4] = "Color Depth:";
				$info['hidden']['image_info']['info']['class'] = "upload_info";
				$info['hidden']['image_info']['info'][1] = $imageInfo[0]." pixels";
				$info['hidden']['image_info']['info'][2] = $imageInfo[1]." pixels";
				$info['hidden']['image_info']['info'][3] = ($imageInfo['channels']==3?"RGB":"CMYK");
				$info['hidden']['image_info']['info'][4] = $imageInfo['bits']." bits";
				
				$info['image'] = "<a href=\"?q=getFullSize$this->currentDirectory/$file\" rel=\"lightbox\" onClick=\"event.stopPropagation();\">
									<img src=\"?q=getThumb$this->currentDirectory/$file\"  width=\"24\" height=\"24\" />
								 </a>";
			else:
				
				$info['image'] = "<img src=\"?q=getThumb$this->currentDirectory/$file\"  width=\"24\" height=\"24\" />";
				
			endif;
		
			$this->ui->addRecord($info);
			$this->row_no++;
			
			unset($info);
			
		endforeach;
		
		$output="</form>";
				
		return $output;
	}
	function getSelectors()
	{
		$output = "<div id=\"hd-uploads_batch_selectors\">Select: <a href=\"javascript:selectAllUploads()\">All</a>, <a href=\"javascript:selectNoneUploads()\">None</a>, <a href=\"javascript:selectUploads('.fileBox')\">Files</a>, <a href=\"javascript:selectUploads('.dirBox')\">Directories</a></div>";
		return $output;
	}
	function getOperations()
	{
		$output = "<div id=\"hd-uploads_batch_operations\">
						<span id=\"batch_move_uploads\">
							<a href=\"javascript:setBatchOp('move')\">Move to <img src=\"".drupal_get_path("module","home_directory")."/images/more.png\" \" alt=\"\"/></a>
						</span>
						<div id=\"hd-move_to_options\">".$this->getMoveToDirs()."</div>
						<span id=\"batch_delete_uploads\">
							<a href=\"javascript:setBatchOp('delete')\" >Delete</a>
						</span>
						<span id=\"batch_download_uploads\">
							<a href=\"javascript:setBatchOp('download')\">Download</a>
						</span>
					</div>";
		return $output;
	}
	function getMoveToDirs()
	{
		if(!empty($this->currentDirectory))
		{
			$cur = explode("/",$this->currentDirectory);
			end($cur);
			unset($cur[key($cur)]);
			$cur = implode("/",$cur);
			if(empty($cur))
				$cur = "/";
			
			$list = "<li onClick=\"batchMoveTo('Up One Level','$cur')\">Up One Level</li>";
		}
		if(!empty($this->moveTo))
		{
			foreach($this->moveTo as $move):
				$list .= "<li onClick=\"batchMoveTo('$move','$this->currentDirectory/$move')\">$move</li>";
			endforeach;
		}
		return $list;
	}
	function getBatchForm()
	{
			return	"<form action=\"?q=uploads/batch\" method=\"post\" name=\"file_list\" id=\"file_list\">
							<input type=\"hidden\" class=\"batchOp\" name=\"operation\">
							<input type=\"hidden\" class=\"moveToDest\" name=\"moveToDest\">";
	}
}
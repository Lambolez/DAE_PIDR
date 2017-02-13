<?php
/*
* Copyright (c) 2004 Heiko Rutenbeck <bzrudi@tuxpower.de>
*
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation; either version 2 of the License, or
*  (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program; if not, write to the Free Software
* Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

if(!defined('TOP_DIR')) { define('TOP_DIR','..'); }


/*
 * Local modification for use on DAE platform
*/
function getOS() {
	return PHP_OS;
}
/*
 * End local modification for use on DAE platform
*/

/**
* create directory recursively - same as "mkdir -p"
*/
function mkdir_p($target,$mode='0700')
{
	if(is_dir($target) || empty($target)) {	// best case check first
		return 1;
	}
	if(file_exists($target) && !is_dir($target)) {	// target exists bug isn't a directory..
		return 0;
	}
	if( mkdir_p( substr( $target,0,strrpos( $target,'/') ) ) )
	{
		return mkdir($target,intval($mode,8)); // crawl back up & create dir tree
	}
	return 0;
}

/**
* copy recursive - same as "cp -R"
*/
function copy_r($source,$dest)
{
	if( !is_dir($source) )
	{
		if( is_file($source) )
		{
			@copy($source,$dest);
			return 1;
		}
		else
		{
			return 0;
		}
	}
	if (!is_dir($dest))  {
		mkdir_p($dest);
	}
	$h=@dir($source);
	while (@($entry=$h->read()) !== false)
	{
		if (($entry!=".")&&($entry!=".."))
		{
			if (is_dir("$source/$entry")&&$dest!=="$source/$entry")
			{
				copy_r("$source/$entry","$dest/$entry");
			}
			else
			{
				@copy("$source/$entry","$dest/$entry");
			}
		}
	}
	$h->close();
	return 1;
}


/**
* returns a unique filename, but different to tempnam() the file doesn't exists
*
* @author  flo
* @param  string  $prefix  prefix for the tmp_file
* @param  string  $suffix  suffix for the tmp_file, formerly the file extension
* @return  string  unique temporary filename
* @uses  many!
* @package  filesys
*
* http://ch.php.net/manual/de/function.uniqid.php
*/
function linpha_tempnam($prefix,$suffix)
{
	$tmp_dir = get_full_path(read_config('tmp_dir'));
	if(!is_writable($tmp_dir)) {
		error_log("linpha_tempnam: tmp_dir ($tmp_dir) isn't writable! Using /tmp instead");
		$tmp_dir = '/tmp';
	}
	/*$tmp_file_org = tempnam($tmp_dir,$prefix);

	if($tmp_file_org=="") {
		error_log("linpha_tempnam($prefix,$suffix): Temporary file cannot be created...");
	}

	if(!unlink($tmp_file_org)) {
		error_log("linpha_tempnam($prefix,$suffix): Temporary file cannot be deleted...");
	}*/
	
	for($i=0;  file_exists($tmp_dir.'/'.$prefix.'_'.session_id().'_'.$i.$suffix);  $i++) { }
	$tmp_file = $tmp_dir.'/'.$prefix.'_'.session_id().'_'.$i.$suffix;
	touch($tmp_file);



	//$tmp_file = $tmp_file_org.$suffix;

	return $tmp_file;
}


/**
* escape string for use in exec() (especially for convert), not in any other function like fopen etc.
*
* @author  flo
* @param  string  $str  string to exec
* @return  string  escape string
* @uses  ?
* @package  filesys
*/
function escape_string($str)
{
	if(getOS() == 'win')
	{
		/**
		 * replace forward slash with backslash, need this for powerarchiver for
		 * downloading albums, because powerarchiver cannot handle forward
		 * slashes in paths, does this disturb somewhere??
		 */
		$str = str_replace("/","\\",$str);

		/**
		* escape string with ("), -> call exec always without (")
		* for example: exec('convert "'.escape_string($input).'" "'.escape_string($output).'") is FALSE !!!
		* escape with (") because under windows we cannot have double quotes in filenames, in contrast to single quotes!
		*/

		$str = '"'.$str.'"';
	}
	else
	{
        /**
         * do not escape string with safemode, because if safemode is on,
         * strings are escaped automatically
         *
         * see http://ch.php.net/manual/en/features.safe-mode.functions.php
         * exec() 	You can only execute executables within the
         * safe_mode_exec_dir. For practical reasons it's currently not allowed
         * to have .. components in the path to the executable. escapeshellcmd()
         * is executed on the argument of this function.
         */

			/**
			* done: maybe we should this replace with the php builtin function
			* escapeshellcmd()
            * 
            * now we should get the same results wether safe mode is on or off 
			*/
			/*$str = str_replace("\\","\\\\",$str);
            
            //$str = escapeshellcmd($str);
			
			$array = Array('"','\'','(',')','[',']','{','}','*','?','!','`','=','&','@',' ',':',';');
		
			foreach($array AS $value)
			{
				$str = str_replace($value,'\\'.$value,$str);
			}*/
		//}
        /**
         * what is the better solution:
         * 1. convert albums/dir\ with\ spaces/test.jpg tmp/test.jpg
         * or
         * 2. convert "albums/dir with spaces/test.jpg" "tmp/test.jpg"
         * 
         * 1. negativ: if safemode is on it doesn't work anymore because the
         * backslashes are escaped too
         */
        $str = str_replace('"','\"',$str);
        $str = '"'.$str.'"';
	}	
	return $str;
}

/**
* gets the file extension from a path/filename.ext, filename.ext, .ext or ext* 
*
* @author  flo
* @param  string  $filename  path to file
* @return  string  fileextension
* @uses  file_is_image(),get_imagetype_from_filename()
* @package  filesys
*/
function get_fileext_from_path($filename)
{
	if(substr_count($filename,'.') == 0) {
		return $filename;	// no point found, -> is already the extension
	}
	$pos = strrpos( $filename, '.' );	// find last point
	$ext = substr( $filename, $pos+1, strlen( $filename ) );	// extract extension
	$ext = strtolower( $ext );	// convert to lowercase
	return $ext;
}

/**
* if $path is absolute, this is returned, if $path is relative, the relative path starting
* from the linpha rootdirectory is returned
*
* @author  flo
* @param  string  $path  a path
* @return  string  a path
* @uses  actions/submit_mod_data.php,photo_cache_cleanup_by_id(),photo_cache_delete_folder()
* @package  filesys
*/
function get_full_path($path)
{
	if(is_absolute_path($path)) {
		$full_path = $path;
	} else {
		$full_path = TOP_DIR.'/'.$path;
	}
	
	return $full_path;
}

/**
* checks wether $path is absolute or relative
*
* @author  flo
* @param  string  $path  a path
* @return  ture|false  is an absolute path or not
* @uses  get_full_path()
* @package  filesys
*/
function is_absolute_path($path)
{
	if(substr($path,0,1) == '/' OR		// string begins with a '/' -> absolut path
		((strpos($path, ':') !== false) AND (strpos( PHP_OS, 'WIN' ) !== FALSE))	// we are on windows and the string contains a ':' -> absolut path (c:/..)
		) {	// absolut path
		return true;
	} else {
		return false;
	}

}

function get_linpha_themes()
{
/*##########################################################
## used in: build_general_conf.php
## returns an array with all theme files (.css) found
## in the folder 'themes'
##########################################################*/
	
	$file_handle = opendir(TOP_DIR."/styles/");
	$file = readdir($file_handle);

	while($file != false)
	{
		$ext = get_fileext_from_path($file);

		// Only files end with '.css' are css files.
		if($ext == "css")
		{
			$pos = strrpos($file,".");
			$name = substr($file,0,$pos);

			$all_themes[$name] = $name;
		}
		$file = readdir($file_handle);
	}

	/* Sort theme files aphabetically */
	asort($all_themes);
	
	return $all_themes;
}

function get_available_language_files()
{
/*##########################################################
## used in: build_general_conf.php
## returns an array with all language files found
## in the folder 'lang'
##########################################################*/

	$folder = get_full_path('lang');

	$file_handle = opendir($folder);
	$file = readdir($file_handle);

	$options = array();
	while($file != false)
	{
		$explode = explode(".",$file);

		/* Only files that starts with 'lang.' are language files. */
		if($explode[0] == "lang")
		{
			$options[$explode[1]] = $explode[1];
		}
		$file = readdir($file_handle);
	}

	/* Sort language files aphabetically */
	asort($options);
	
	return $options;
}

function compare_array_with_folder($array, $folder)
{
/*##########################################################
## used in: first_lev_funct()
## return 1 if there were differences found, 0 if not
##########################################################*/

	$handle=opendir('./'.$folder);	// maybe we should use TOP_DIR ..?
	while (false !== ($file = readdir ($handle)))
	{
		if(is_dir('./'.$folder.'/'.$file) &&
			$file != "." && $file != ".." && $file != "CVS")
		{
			if(in_array($file, $array))
			{
				$key = array_search($file, $array);
				$array[$key] = '';
			} else {
				return 1;	// new folder
			}
		}
	}
	closedir($handle);
	foreach($array AS $value) {
		if($value!="") {
			return 1;	// folder deleted
		}
	}
	return 0;

}

function get_file_md5sum($src_file)
{
/*##########################################################
## used in: db_api.php while creating thumbnails
## This function calculates the md5sum for a file.
## returns:
## md5sum
##
## this function is the same like md5_file()  (PHP 4 >= 4.2.0)
## 
##########################################################*/

	/*
	this produce bad results if php < 4.3 or magic_quotes_runtime set to 1 !!!!!!!!
	
	In PHP 4.3.0 file() became binary safe. (http://ch.php.net/manual/en/function.file.php) !!
	
	old function:
	$filecontent = implode("", file($src_file));		// this produces bad results because file() was in php < 4.3 not binary safe
	return md5($filecontent);							// this produces bad results we haven't checked if magic_quotes are on
	*/

	if($handle = @fopen ($src_file, "rb"))	// open as binary (otherwise, we get a wrong md5sum)
	{
		$contents = fread ($handle, filesize ($src_file));
		fclose ($handle);
		
		if(get_magic_quotes_runtime())
		{
			$contents = stripslashes($contents);
		}
		
		return md5($contents);
	}
	else
	{
		return '';
	}
}

function nice_filesize($filesize, $precision)
{
/*##########################################################
## used in: sql.php/video_site.php/img_view.php (download link)
## This function formats the filesize (byte) value to kB/MB/GB
## returns:
## formatted file size
#############################################################*/
	
	if ($filesize < 1000) {
		$sizeunit  = 'bytes';
		$precision = 0;
	} else {
		$filesize /= 1024;
		$sizeunit = 'kB';
	}
	if ($filesize >= 1000) {
		$filesize /= 1024;
		$sizeunit = 'MB';
	}
	if ($filesize >= 1000) {
		$filesize /= 1024;
		$sizeunit = 'GB';
	}
	return number_format($filesize, $precision).' '.$sizeunit;
}



function readfolder($folder, $text_nothing_found, $arr_filetypes, $return_file_exts)
{
/*##########################################################
## used in: watermark.php
## Parameter:
## - $folder: path to folder (can be relativ)
## - $text_nothing_found: put this text in the array if nothing were found, leave blanc to disable
## - $arr_filetypes: array with fileextensions that will be filtered
## - $return_file_exts: 0,1 return full filename with extensions or not
##
## returns:
## - Array with all filenames found
##########################################################*/

	if($file_handle = @opendir("$folder"))
	{
		$file = readdir($file_handle);
		while($file != false)
		{
			$explode = explode(".",$file);
			foreach($arr_filetypes as $value)
			{
				if(@$explode[1] == $value)
				{
					if($return_file_exts)
					{
						$arr_folder_content[$file] = $file;
					} else {
						$arr_folder_content[$explode[0]] = $explode[0];
					}
				}
			}
			$file = readdir($file_handle);
		}
		asort($arr_folder_content);
	}
	if(!isset($arr_folder_content) && $text_nothing_found!="")
	{
		$arr_folder_content['no'] = $text_nothing_found;
	}
	return $arr_folder_content;
}

/*#######################
## outdated functions! ##
#######################*/

/*function check_string_if_secure_to_exec($string, $arr_allowed)
{
/*##########################################################
## used in: convert_image_convert()
## Check if the string contains malicious code that would be executed
## This function should always be used if the function exec() is used
## $arr_allowd: array with allowed signs
## example: array("|" => 1) -> the sign "|" is allowed one time
## example: array() -> nothing is allowed
## returns:
## check true/false
##
## outdated, not used anymore..!! use escape_string() or check_variable() instead!
##########################################################

	//$array_mal_signs = array(";","|","&","rm *");
	$array_mal_signs = array(";","|","rm *");

	// check if there is more than allowed
	while(list($key,$value) = each($arr_allowed)) {
		//echo 'key: '.$key.' value: '.$value.'<br/>';
		if(substr_count($string, $key)>$value) {
			die("Shut up!!! to much");
		}
	}

	// check for other bad things
	foreach($array_mal_signs as $value) {
		if(strpos($string, $value) && !array_key_exists($value, $arr_allowed)) { // the allowed stuff is checked before
			die("Shut up!!! not allowed");
		}	
	}
	return true;
}*/

/*function find_file_in_folder($ext,$foldername)
{
/*##########################################################
## used in: benchmark.php
## Searches for a file with extension $ext recursivly in folder $foldername
## Until the first file is found, the fullpath is returned, otherwise ""
## returns:
## fullpath
##
## outdated, not used anymore!
##########################################################

	if($file_handle = opendir($foldername))
	{
		while($file = readdir($file_handle))
		{
			if(is_dir($foldername."/".$file)) {
				if($file!="." && $file!="..") {
					$string = find_file_in_folder($ext,$foldername."/".$file);
					if($string!="") {
						return $string;
					}
				}
			} else {
				if($pos_ext = strrpos($file,".")) {
					if($ext == substr($file,$pos_ext+1,strlen($file)-$pos_ext-1)) {
						return $foldername."/".$file;
					}
				}
			}
		}
	}
	return "";
}*/




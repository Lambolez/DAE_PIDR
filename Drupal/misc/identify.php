<?php
/*
* Copyright (c) 2004 Heiko Rutenbeck <hide@address.com>
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

/*
 * Local modification for use on DAE platform
 */

require_once(dirname(__FILE__).'/filesys.php');

global $config;
$config['use_convert'] = true;
$config['convert_path'] = '/opt/csw/bin/';

function read_config($name) {
  global $config;

  return $config[$name];
}
/*
 * End local modification
 */


/**
* all about getting information about images and videos
*
* @package  image
* @subpackage identify
*/


/**
* returns true if this is a panorama image
* all panorama images have 2/1 ratio (width/height) so link is only enabled if == true
*
* @uses  img_view.php, file_download.php
* @return  true|false
*/
function is_panorama_image($org_width,$org_height)
{
	if( ($org_width/2) >= $org_height &&
		$org_width > 0 && $org_height > 0 && !empty($org_width) && !empty($org_height) )
	{
		return true;
	} else {
		return false;
	}
}

/**
* takes care if we are using gdlib or imagemagick
* returns true if we can read this kind of image
*
* @author  flo
* @param  int  imagetype in the format of getimagesize() "JPEG", ...
* @return  int  true|false
* @uses  actions/file_download.php, make_thumbs()
*/
function is_supported_image($org_type)
{
/*##########################################################
## used in: file_download.php and make_thumbs()
## checks if this file is an image and therefore needs a watermark
## return 1 if this filename is an image, otherwise 0
##########################################################*/
	//error_log("i got my type as : $org_type");	
	if(read_config('use_convert')) {
		if($org_type >= 1 && $org_type < 1000) {
			return true;
		} else {
			return false;
		}
	} else {	// gdlib
		if($org_type >= 1 && $org_type <= 3) {	// gdlib supports only jpg, gif and png!
			return true;
		} else {
			return false;
		}
	}
}

/**
* checks if this file is a video
*
* @author  flo
* @param  int  imagetype in the format of getimagesize()
* @return  true|false
* @uses  make_thumbs(), file_download()
*/
function is_video($org_type)
{
	if($org_type >= 1000 && $org_type < 2000)
	{
		return true;
	} else {
		return false;
	}
}

/**
* converts the image type from imagemagick to the format of getimagesize()
* for example: "TIFF" (imagemagick) -> "7" (getimagesize())
*
* @author  flo
* @param  string  $type  imagetype from identify, for example "TIFF", "JPEG", ...
* @param  string  $path  optional, path to image, needed for tifftype detection
* @return  int  imagetype in the format of getimagesize()
* @uses  linpha_getimagesize()
*/
function convert_imagetype($type,$path='')
{
	$type = trim($type);
	$type = strtolower($type);

	$imagetypes = get_imagetype_translation_array();

	if(array_key_exists($type,$imagetypes)) {
		$orgtype = $imagetypes[$type];
	} else {
		$orgtype = $imagetypes['unsupported'];
	}

	/**
	* there are two different tiff formats
	* check which tiff file it is
	*/
	if( $orgtype == 7 )
	{
 		if( file_exists( $path ) ) {	// valid
			$orgtype = detect_tifftype($path);
		} else {
			$orgtype = 7;	// set default to number 7
		}
	}
	
	return $orgtype;
}

/**
* detects the tiff type of the image
* there are two different TIFF formats:
* intel byte order (little endian) and motorola byte order (big endian))
* 
* The second word of TIFF files is the TIFF version number, 42, which has
* never changed. The TIFF specification recommends testing for it
*
* @author  bzrudi?
* @param  string  $path  path to tiffimage
* @return  int  7 or 8 (imagetype in the format of getimagesize())
* @uses  convert_imagetype()
*/
function detect_tifftype($path)
{
	$tfh = fopen( $path, "r" );
	$magic = fread( $tfh, 4 );
	fclose( $tfh );
	
	// big-endian=8, little-endian=7 according to getimagesize() of PHP4.2
	if(ord($magic[2])==42 && ord($magic[3])==0) {
		$type = 7;
	} else {
		$type = 8;
	}
	return $type;
}

/**
* Get image properties.
* This function extends PHP's getimagesize() function. This include's
* support for TIFF like files with PHP < 4.2.0 and others
* 
* @author  bzrudi,flo
* @param  string  $path  path to image
* @return  array  array(width,height,type,img_string)
* @uses  many!
*/
function linpha_getimagesize($path)
{
	$g = getimagesize( $path );
	
	$ext = get_fileext_from_path($path);
	
	/**
	* $g[2] doesn't exists: -> image type is for example TIFF, but it also can be AVI etc.
	* -> use identify from imagemagick to determine the imagetype
	*
	* but first, check if it is a video, because imagemagick's identify really sucks in detecting videos
	* (segmentation faults and wait times about 2 minutes for each file!)
	* test results:
	* version 5.4.4, 5.5.6, 5.5.7 slow (for one AVI file 2 minutes)
	* 6.0.0 works fine
	* 6.0.7 segmentation fault
	*/
	if( !$g[2] )
	{
		$org_type = convert_imagetype($ext);
		if(is_video($org_type))
		{
			$g[2] = $org_type;
		}
		else
		{
			$identify_path=read_config('convert_path') ."identify";
			if( read_config('use_convert') )
			{
				/**
				* $array_output[0] contains something like '2272 1704 JPEG'
				*/
				exec ( $identify_path.' -format "%w %h %m" '.escape_string($path.'[0]').' 2> /dev/null', $array_output, $return_value);
				
				if($return_value == 0)	// identify succeed
				{
					$g = explode( " ", $array_output[0] );
				
					$g[2] = convert_imagetype($g[2],$path);
				}
			}
		}
	}
	else	//  getimagesize detects mpg and mp4 as type 15, but type 15 is wbmp (look at the mimetype returned by getimagesize), pay attention...
	{
		if($g[2] == 15)
		{
			if(convert_imagetype($ext) == 1000)	// 1000 = mpg, mpeg, mpe, ...
			{
				$g[2] = convert_imagetype('mpg');	// get imagetype of mpg
			}elseif($ext == 'mov'){
				$g[2] = convert_imagetype('mov');
			}elseif($ext == 'mp4'){
				$g[2] = convert_imagetype('mp4');
			}
		}
	}
	
	/**
	* still don't have an imagetype, take imagetype by fileextension
	* @todo  what should we do now, getimagesize and identify doesn't handle this files as images and we doesn't know the image height and width, i think they therefore can't convert/handle this file, so we should declare it as unsupported. But we need still a check if it is a video or something other, so we can't remove get_imagetype_from_filename(). That means we should check if get_imagetype_from_filename() returned a value lesser than 1000 and if so, delete it! what do others mean?! flo
	*/
	if( !$g[2] )
	{
		$type = get_imagetype_from_filename($path);
		if($type < 1000)
		{
			$g[2] = convert_imagetype('unsupported');
		}
		else
		{
			$g[2] = $type;
		}
	}
	
	
	/**
	* set to not empty to prevent erros on commands like
	* list($org_width,$org_height,$org_type) = linpha_getimagesize($img2rotate);
	*/
	if(!isset($g[0]))
	{
		$g[0] = 0;
	}

	if(!isset($g[1]))
	{
		$g[1] = 0;
	}
	
	$g[3] = 'width="'.$g[0].'" height="'.$g[1].'"';

	return $g;
}

function get_mime_by_ext($ext)
{
/*##########################################################
## used in: file_download.php
## get the mime-type by file extension
##########################################################*/
	switch($ext) {
		case "avi": $ctype="video/avi"; break;
		case "mpg": $ctype="video/mpeg"; break;
		case "mpeg": $ctype="video/mpeg"; break;
		case "mp4": $ctype="video/quicktime"; break;
		case "mov": $ctype="video/quicktime"; break;
	
		case "bmp": $ctype="image/bmp"; break;
		case "gif": $ctype="image/gif"; break;
		case "png": $ctype="image/png"; break;
		case "jpg": $ctype="image/jpg"; break;
		case "jpeg": $ctype="image/jpg"; break;
		case "psd": $ctype="image/psd"; break;
		case "tif": $ctype="image/tiff"; break;
		case "tiff": $ctype="image/tiff"; break;
	
		case "tar": $ctype="application/x-tar"; break;
		case "gz": $ctype="application/x-gzip"; break;
		case "zip": $ctype="application/zip"; break;
	
		default:    $ctype="application/force-download"; break;
	}
	return $ctype;
}

/**
* gets the imagetype from a path/filename.ext, filename.ext, .ext or ext* 
*
* @author  flo
* @param  string  $filename  path to image
* @return  int  imagetype in the format of getimagesize()
* @uses  ?
*/
function get_imagetype_from_filename($filename)
{
	$ext = get_fileext_from_path($filename);
	$imagetypes = get_imagetype_translation_array();
	if(array_key_exists($ext,$imagetypes)) {
		return $imagetypes[$ext];
	} else {
		return $imagetypes['unsupported'];
	}
}

/**
* gets an array with all imagetypes, fileext are the keys, imagetype the values
* see http://ch.php.net/manual/de/function.exif-imagetype.php
*
* @author  flo
* @return  array  keys=>value, fileext=>imagetype
* @uses  get_imagetype_from_filename(),convert_imagetype()
*/
function get_imagetype_translation_array()
{
	$array = Array(
// getimagesize starts here

	'gif' => 1,
	'jpg' => 2,
	'jpeg' => 2,
	'png' => 3,
	'swf' => 4, 	//not supported by imagemagick?
	'psd' => 5,
	'bmp' => 6,
	'tiff' => 7,
	'tiff_ii' => 7,	// IMAGETYPE_TIFF_II (intel byte order) little endian
	'tiff_mm' => 8,	// IMAGETYPE_TIFF_MM (motorola byte order) big endian
	'jpc' => 9,
	'jp2' => 10,
	'jpx' => 11,	//not supported by imagemagick?
	'jb2' => 12,	//not supported by imagemagick?
	'swc' => 13,	//not supported by imagemagick?
	'iff' => 14,	//not supported by imagemagick?
	'wbmp' => 15,	// getimagesize detects mpg as type 15, but type 15 is wbmp (look at the mimetype returned by getimagesize), pay attention...
	'xbm' => 16,


// imagemagick starts here
	'art' => 100,
//	'avi' => 101,	video
	'avs' => 102,
//	'bmp' => 103,	already
	'cgm' => 104,
	'cin' => 105,
	'cmyk' => 106,
	'cur' => 107,
	'cut' => 108,
	'dcm' => 109,
	'dcx' => 110,
	'dib' => 111,
	'dpx' => 112,
	'emf' => 113,
	'epdf' => 114,
	'epi' => 115,
	'eps' => 116,
//	'eps2' => 117,	
//	'eps3' => 118,	
	'epsf' => 119,
	'epsi' => 120,
	'ept' => 121,
	'fax' => 122,
	'fig' => 123,
	'fits' => 124,
	'fpx' => 125,
//	'gif' => 126,	already
	'gplt' => 127,
	'gray' => 128,
	'hpgl' => 129,
//	'html' => 130,	this type shouldn't be handled as image
	'ico' => 131,
	'jbig' => 132,
	'jng' => 133,
//	'jp2' => 134,	already
//	'jpc' => 135,	already
//	'jpeg' => 136,	already
	'man' => 137,
	'mat' => 138,
	'miff' => 139,
	'mono' => 140,
	'mng' => 141,
//	'm2v' => 142,	video
//	'mpeg' => 143,	video
	'mpc' => 144,
	'msl' => 145,
	'mtv' => 146,
	'mvg' => 147,
	'otb' => 148,
	'p7' => 149,
	'palm' => 150,
	'pbm' => 151,
	'pcd' => 152,
	'pcds' => 153,
	'pcl w' => 154,
	'pcx' => 155,
	'pdb' => 156,
	'pdf' => 157,	
	'pfa' => 158,
	'pfb' => 159,
	'pgm' => 160,
	'picon' => 161,
	'pict' => 162,
	'pix' => 163,
//	'png' => 164,	already
	'pnm' => 165,
	'ppm' => 166,
	'ps' => 167,
	'ps2' => 168,
	'ps3' => 169,
//	'psd' => 170,	 already
	'ptif' => 171,
	'pwp' => 172,
	'rad' => 173,
	'rgb' => 174,
	'rgba' => 175,
	'rla' => 176,
	'rle' => 177,
	'sct' => 178,
	'sfw' => 179,
	'sgi' => 180,
	'shtml' => 181,
	'sun' => 182,
	'svg' => 183,
	'tga' => 184,
//	'tiff' => 185,	already
	'tim' => 186,
	'ttf' => 187,
//	'txt' => 188,	this type shouldn't be handled as image
	'uil w' => 189,
	'uyvy' => 190,
	'vicar' => 191,
	'viff' => 192,
//	'wbmp' => 193,	already
	'wmf' => 194,
	'wpg' => 195,
//	'xbm' => 196,	already
	'xcf' => 197,
	'xpm' => 198,
	'xwd' => 199,
	'yuv' => 200,

// video
	'mpg' => 1000,	// getimagesize detects mpg as type 15, but type 15 is wbmp (look at the mimetype returned by getimagesize), pay attention...
	'mpeg' => 1000,
	'mp4' => 1001,
	'avi' => 1002,
	'mov' => 1003,
	'm2v' => 1004,
	'wmv' => 1005,
	'mpe' => 1000,
/*
// sound
	'mp3' => 2000,
	'wav' => 2001,
	'ogg' => 2002,
	'midi' => 2003,
*/

/*
//text
	'html' => 3000,
	'htm' => 3001,
	'php' => 3002,
	'asp' => 3003,

	'txt' => 4000,
	'pdf' => 4001,
	'rtf' => 4002,
	
// OpenOffice
	'sxw' => 5000,	// OpenOffice Textdocument
	'stw' => 5001,	// OpenOffice Textdocument
	'sxc' => 5002,	// OpenOffice Spreadsheet
	'stc' => 5003,	// OpenOffice Spreadsheet
	'sxi' => 5004,	// OpenOffice Presentation
	'sti' => 5005,	// OpenOffice Presentation
	'sxd' => 5006,	// OpenOffice Draw
	'std' => 5007,	// OpenOffice Draw
	'sxm' => 5008,	// OpenOffice Formula
	
	'sdw' => 5020,	// Starwriter Textdocument
	'sdc' => 5021,	// Starcalc
	'sda' => 5022,	// StarImpress
	'sdd' => 5023,	// StarImpress
	'sdp' => 5024,	// StarImpress

// MS Office
	'doc' => 5500,	// MS Word
	'dot' => 5501,	// MS Word
	'xls' => 5502,	// MS Excel
	'xlt' => 5503,	// MS Excel
	'ppt' => 5504,	// MS Powerpoint
	'pps' => 5505,	// MS Powerpoint
	'pot' => 5506,	// MS Powerpoint
*/

	'unsupported' => 9999999

	);
	return $array;
}



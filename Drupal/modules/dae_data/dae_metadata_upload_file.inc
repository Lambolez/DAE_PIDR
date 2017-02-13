<?php
/**
 * @file
 * 
 * @author  Bart Lamiroy <lamiroy@cse.lehigh.edu>
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

require_once("./".drupal_get_path('module','home_directory')."/home_directory.module");
 
/**
 * Implementation of upload.
 */
function dae_metadata_upload() {
  global $user;

  $form = array();
  $form['#attributes'] = array('enctype' => "multipart/form-data");
  
  $form['data_file'] = array(
    '#type' => 'file',
    '#title' => t('Meta Data & Annotations File'),
    '#description' => t('Description file to upload. Beware upload size is limited to 1G. For larger files, please contact dae-admin[@]cse.lehigh.edu.')
  );
  $form['data_copyright'] = array(
    '#type' => 'file',
  	'#type' => 'hidden', /* temporarily */
  	'#title' => t('Copyright Description File'),
    '#description' => t('Copyright description file to upload (optional)')
  );
  $form['data_description_url'] = array(
    '#type' => 'textfield',
  	'#type' => 'hidden', /* temporarily */
    '#title' => t('External URL for Dataset Description'),
    '#description' => t('Dataset description URL to reference (optional)')
  );
  $form['data_submit'] = array(
    '#type' => 'submit',
    '#title' => t('Submit'),
    '#default_value' => t('Upload'),
    '#description' => t('Submit')
  );
  return $form;
}

/**
 * 
 * Creates a directory with a unique name at the specified with the specified prefix.
 * (copied from http://php.net/manual/en/function.tempnam.php)
 * 
 * @param unknown_type $path
 * @param unknown_type $prefix
 * 
 * @return directory name on success, false otherwise
 * 
 */
function tmpdir($path, $prefix)
{
        // Use PHP's tmpfile function to create a temporary
        // directory name. Delete the file and keep the name.
        $tempname = tempnam($path,$prefix);
        if (!$tempname)
                return false;

        if (!unlink($tempname))
                return false;

        // Create the temporary directory and returns its name.
        if (mkdir($tempname))
                return $tempname;

        return false;
}


/**
 * Implementation of upload submit.
 */
function dae_metadata_upload_submit($form_id, $form_values)
{
  global $user;
    
  $fileValidators = array(
    'file_validate_extensions' => array('xml txt'),
  );
  $documentValidators = array(
    'file_validate_extensions' => array('doc txt docx pdf odt htm html'),
  );
  
  $dir = variable_get('dae_data_upload_dir', '/dae/upload/') . '/' . $user->uid . '/' . date("Ymd") . '/';
  
  if(!is_dir($dir)) {
    //drupal_set_message(t('Pathname %path non existant', array('%path' => $dir)), 'error');
	mkdir($dir,0777,true);
  }

  $file = file_save_upload('data_file', $fileValidators);
  $copyrightFile = file_save_upload('data_copyright', $documentValidators);
  
  if($file)
  {     
	  $filename=preg_replace("/[^a-zA-Z0-9\.\ ]/","_",$file->filename);  
	  $filename = home_directory_upload_rename($dir,$filename);
	  
	  $nameString = tmpdir($dir, 'XML');
	  $xmlFilename = $nameString . '_metadata.xml';
	  rename($file->destination, $xmlFilename);	  
  }
  else {
      //drupal_goto('uploads/error_page');
	  drupal_set_message(t('Upload Failed'),'error');
      return;
  }

  if($copyrightFile)
  {  
	  $filename=preg_replace("/[^a-zA-Z0-9\.\ ]/","_",$copyrightFile->filename);  
	  $filename = home_directory_upload_rename($dir,$filename);
	  
	  $copyrightFileName = $dir . $filename;
	  rename($copyrightFile->destination, $copyrightFileName);	  
  }
  
  $userId = getPID($user->uid);
        
  $command = escapeshellarg(variable_get('dae_data_upload_script','commit.sh'));
  $server = escapeshellarg(variable_get('dae_db_server','localhost'));
  $SID = escapeshellarg(variable_get('dae_db_name','DAE'));
  $mail = escapeshellarg($user->mail);
      
  $commandString = "$command $nameString $server $SID $mail > $nameString" . "_serverLog 2>&1 &";
  $returnString = system($commandString, $returnValue);
      
  if($returnValue) {
      shell_exec("rm '$xmlFilename' '$copyrightFileName'");
      drupal_set_message(t('Commit Error: '.$returnString),'error');
      return;
  }
      
  drupal_set_message('<p>First upload stage succeeded.</p>
      		<p>Now switching to second stage: an e-mail notification will be sent upon completion.</p>
      		<p>You can now close this window or upload another file.</p>');
}

function dae_metadata_upload_page($form_state = NULL)
{
    $output = "<br/><h3>Disclaimer</h3>";
    $output .= "<p>By uploading data with this form you certify having read <a href=\"uploadandcopyright\">data upload and copyright information</a> and you agree with the <a href=\"termsofuse\">general terms of use</a> and attest that:</p>\n";
    
    $output .= "<ul>
    				<li>you understand that by uploading data, you make it available to all users of this platform;</li>
    				<li>you will not upload data irrelevant to the domains of document analysis research and machine perception in general</li>
    				<li>you remain sole legal responsible for the data you have uploaded, especially, but not restricted to, its Copyright
    				    and rights to distribute to the research community, its nature and respect of laws and regulations.</li>
    				<li>you understand that Lehigh University does not offer any guarantee with respect to availabilty, integrity or perennity of the stored data or bandwith; Lehigh University has the right to remove any data form this platform, interrupt or suspend access to the platform without any notice.</li>    
    			</ul>";
    
    $output .= "<h3>Upload Information</h3>";

    $output .= '<p>Please be aware that the upload is only one part of the whole registration process:
    				<ol>
    					<li>First, use this form to upload your dataset. This upload process takes between a few seconds upto a few minutes, depending on the file volume and bandwith.</li>
    					<li>Second, our pre-parser checks for obvious problems with your archive and dataset name. Success of failure are immediately reported on this web page.</li>
    					<li>Finally, a background process parses and registers your uploaded data. This can take a significant amount of time (upto 1 day, depending on the size of the dataset) before the actual data shows up on the platform.
    					E-mail confirmation is sent on completion.</li>
    				</pl>
    			</p>';
    
    $output .= drupal_get_form('dae_metadata_upload');
    
    return $output;
}

function dae_metadata_upload_validate($form_id, $form_values)
{
     return true;
}
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
 * 
 * This function is mainly the definition of the Drupal form that serves as an entry point for
 * uploading a document and annotation collection. It principal role is to handle a number of 
 * descriptions of the dataset on the one hand, to upload the document archive and copyright notice.
 * 
 * The actual handling of the data is done in dae_data_upload_submit().
 * 
 * @NOTE: this form is fairly rudimentary and rather bulky. 
 * It copies the TC10-TC11 (www.iapr-tc10.org/ and www.iapr-tc11.org/) upload forms.
 * 
 * @todo: one day this filling out of this form should be made more streamlined.
 * 
 */
function dae_data_upload() {
  global $user;

  $form = array();
  $form['#attributes'] = array('enctype' => "multipart/form-data");
  
  $form['data_dataset_name'] = array(
    '#type' => 'textfield',
    '#required' => true,
    '#title' => t('Dataset Name'),
    '#description' => t('Name by which this dataset should be referenced (duplicate names will be rejected)')
  );
  $form['data_file'] = array(
    '#type' => 'file',
    '#title' => t('Dataset Archive'),
    '#description' => t('Archive to upload. Beware upload size is limited to 1G. For larger files, please contact dae-admin[@]cse.lehigh.edu.')
  );
  $form['data_url'] = array(
    '#type' => 'textfield',
    '#title' => t('Dataset URL'),
    '#description' => t('URL of file to retrieve. Beware upload size is limited to 1G. For larger files, please contact dae-admin[@]cse.lehigh.edu.')
  );
  $form['data_copyright'] = array(
    '#type' => 'file',
    '#title' => t('Copyright Description File'),
    '#description' => t('Copyright description file to upload (optional)')
  );
  $form['data_copyright_url'] = array(
    '#type' => 'textfield',
    '#title' => t('Copyright Description URL'),
    '#description' => t('Copyright description url (optional)')
  );
  $form['data_dataset_short'] = array(
    '#type' => 'textfield',
    '#title' => t('Short Dataset Description'),
    '#description' => t('Short description of this dataset (its use, content, etc.)')
  );
  $form['data_description'] = array(
    '#type' => 'file',
    '#type' => 'hidden', /* temporarily */
    '#title' => t('Dataset Description File'),
    '#description' => t('Dataset description file to upload (optional)')
  );
  $form['data_description_url'] = array(
    '#type' => 'textfield',
  	'#type' => 'hidden', /* temporarily */
    '#title' => t('External URL for Dataset Description'),
    '#description' => t('Dataset description URL to reference (optional)')
  );
  
  $form['description_field'] = array(
  	'#type' => 'fieldset',
  	'#title' => t('Detailed Description'),
  	'#weight' => 5,
    '#tree' => TRUE,
  	'#collapsible' => TRUE,
  	'#collapsed' => TRUE
  );
  
  $form['description_field']['keywords'] = array(
  	'#type' => 'textfield',
  	'#title' => t('Keywords'),
    '#description' => t('Please provide a list of keywords that will be used to categorise and search for your
dataset.'),
  );
  
  $form['description_field']['detailed_description'] = array(
    '#type' => 'textarea',
    '#title' => t('Description'),
    '#description' => t('Write a short description (a couple of paragraphs max) of what is included in the
dataset. Importantly, include information about how the dataset was constructed and the
data collected. For example if 100 scanned pages are submitted, it is important to know if they
are sourced from a single book, or they are 100 pages from 100 distinct books. If data were
collected from human subjects describe the task they were given, and any statistics that you
find relevant (e.g. nationality, mother tongue, age groups, gender etc).')
  );
  
  $form['description_field']['metadata'] = array(
    '#type' => 'textarea',
    '#title' => t('Metadata and Technical Details'),
    '#description' => t('Please describe here any metadata that will be submitted along
with the dataset and how they are encoded. Any naming conventions or internal structure
used should be described here. Also, include information (as applicable) regarding the number
of samples in the dataset, the file formats used etc. Also, please include information about the
total size of the dataset in question, this will help us identify the best way to upload the data.')
  );
  
  $form['interpretation_field'] = array(
  	'#type' => 'fieldset',
  	'#title' => t('Interpretation Information'),
  	'#weight' => 6,
    '#tree' => TRUE,
  	'#collapsible' => TRUE,
  	'#collapsed' => TRUE
  );
  
  $form['interpretation_field']['new_interpretation'] = array(
  	'#type' => 'radios',
  	'#title' => t('Are you submiting a interpretation information along with the dataset?'),
  	'#description' => t('If you do, then please fill in
this section otherwise proceed directly to the next one.'),
  	'#default_value' => 1,
    '#options' => array(t('Yes'), t('No')),
  );
  
  $form['interpretation_field']['title'] = array(
  	'#type' => 'textfield',
  	'#title' => t('Title'),
    '#description' => t('Please provide a short title for your Interpretation data (e.g. �Skew Angles for the
ACRONYM dataset� or �Layout and Transcription for the ACRONYM dataset�).'),
  );
  
  $form['interpretation_field']['keywords'] = array(
  	'#type' => 'textfield',
  	'#title' => t('Keywords'),
    '#description' => t('Please provide a list of application domains / research tasks where these
Interpretation data could be useful for (e.g. layout-analysis, character recognition, text
detection, word spotting).'),
  );
  
  $form['interpretation_field']['detailed_description'] = array(
    '#type' => 'textarea',
    '#title' => t('Description'),
    '#description' => t('Please provide a short description about the Interpretation data submitted. When
Interpretation data were created manually, the ground-truthing protocol/guidelines should be
included here. In the case of synthetic data, any parameters used should be listed here. Also
describe the Interpretation format used to store the data and if applicable include a sample
file. Please include information about the total size of the Interpretation data.')
  );
  
  $form['research_field'] = array(
  	'#type' => 'fieldset',
  	'#title' => t('Research Tasks'),
  	'#weight' => 7,
    '#tree' => TRUE,
  	'#collapsible' => TRUE,
  	'#collapsed' => TRUE
  );
  
  $form['research_field']['new_research_task'] = array(
  	'#type' => 'radios',
  	'#title' => t('Do you want to define any Research Tasks based on the dataset and/or Interpretation data
you have submitted?'),
  	'#default_value' => 1,
  	'#description' => t('We strongly encourage you to define as a research task the research
question you set out to answer in your related publications. If you do not want to define any
tasks, then please proceed directly to the next section.'),
    '#options' => array(t('Yes'), t('No')),
  );
  
  $form['research_field']['title'] = array(
  	'#type' => 'textfield',
  	'#title' => t('Task Title'),
    '#description' => t('Please provide a title for the research task you propose to the community
(e.g. �Character Recognition in Typewritten Historical Documents�).'),
  );
  
  $form['research_field']['detailed_description'] = array(
    '#type' => 'textarea',
    '#title' => t('Task Description'),
    '#description' => t('A short description of the task you propose: what is the purpose of this, its
importance to the future of the community, the state of the art at the time of defining this,
citations to any existing publications treating this problem with this or other datasets, etc.')
  );
  
  $form['software_field'] = array(
  	'#type' => 'fieldset',
  	'#title' => t('Software'),
  	'#weight' => 9,
    '#tree' => TRUE,
  	'#collapsible' => TRUE,
  	'#collapsed' => TRUE
  );
  
  $form['software_field']['new_software'] = array(
  	'#type' => 'radios',
  	'#title' => t('Do you intend to submit any accompanying software?'),
  	'#default_value' => 1,
    '#options' => array(t('Yes'), t('No')),
  );
  
  $form['software_field']['detailed_description'] = array(
    '#type' => 'textarea',
    '#title' => t('Description'),
    '#description' => t('Describe briefly the functionality of the submitted software. Consider sending a
thumbnail of the software along with the form.')
  );
  
  $form['software_field']['technical_details'] = array(
    '#type' => 'textarea',
    '#title' => t('Technical Details'),
    '#description' => t('Please include information regarding minimum requirements, operating
system needed, any special installation instructions etc.')
  );
  
  $form['other_field'] = array(
  	'#type' => 'fieldset',
  	'#title' => t('Other Material'),
  	'#weight' => 10,
    '#tree' => TRUE,
  	'#collapsible' => TRUE,
  	'#collapsed' => TRUE
  );
  
  $form['other_field']['related_publications'] = array(
    '#type' => 'textarea',
    '#title' => t('Related Publications'),
    '#description' => t('Please provide a list of published contributions that relate in some way to
this dataset (they introduce/describe the dataset; they make use of it for training or evaluation
etc).')
  );
  
  $form['other_field']['submitted_files'] = array(
    '#type' => 'textarea',
    '#title' => t('Submitted Files'),
    '#description' => t('Please provide a list of files that you submit along with this form. These could be
samples from the dataset, sample Interpretation files, screenshots of the submitted software,
etc. We strongly advice you to submit sample files for every aspect of the submission you
describe in this form. Also try to send screenshots or other representative images that we can
use in the Web site to illustrate the material submitted.')
  );
  
  $form['copyright_field'] = array(
  	'#type' => 'fieldset',
  	'#title' => t('Copyright Ownership'),
  	'#weight' => 11,
    '#tree' => TRUE,
  	'#collapsible' => TRUE,
  	'#collapsed' => TRUE
  );
  
  $form['copyright_field']['author'] = array(
    '#type' => 'textarea',
    '#title' => t('Contact Author'),
    '#description' => t('Give the name and contact details of the person responsible for this submission.
This information will be published online along with the material submitted.')
  );
    
  $form['copyright_field']['own_copyright'] = array(
  	'#type' => 'radios',
  	'#title' => t('Do you own the copyright to all the material you have submitted?'),
    '#options' => array(t('Yes'), t('No')),
  );
  
  $form['copyright_field']['answered_no'] = array(
    '#type' => 'textarea',
    '#title' => t('If you answered No to the question above, please let us know who owns the copyright and why
you think you are able to place this data in the public domain (e.g. if the material is already
publicly available under a creative-commons license).'),
  );
  
  $form['data_submit'] = array(
    '#type' => 'submit',
    '#title' => t('Submit'),
    '#default_value' => t('Upload'),
    '#description' => t('Submit'),
  	'#weight' => 12,
  );
  
  return $form;
}


/**
 * Implementation of upload submit.
 */
function dae_data_upload_submit($form_id, $form_values)
{
  global $user;
    
  $fileValidators = array(
    'file_validate_extensions' => array('zip tar tgz rar'),
  );
  $documentValidators = array(
    'file_validate_extensions' => array('doc txt docx pdf odt htm html'),
  );
  
  /* 
   * Construction of the upload directory.
   * 
   * Uploads are hierarchically ordered according the User ID values and upload date.
   * 
   * @bug: because of its naming structure, the same user uploading identical file names on the same day may cause naming collisions.
   * 
   */ 
  $dir = variable_get('dae_data_upload_dir', '/dae/upload/') . '/' . $user->uid . '/' . date("Ymd") . '/';
  
  if(!is_dir($dir)) {
	mkdir($dir,0777,true);
  }

  /*
   * Uploading the files from the form into the temporary Drupal upload directory 
   * (this location is *NOT* directory $dir previously defined)
   * 
   */
  $file = file_save_upload('data_file', $fileValidators);
  $copyrightFile = file_save_upload('data_copyright', $documentValidators);
  $descriptionFile = file_save_upload('data_description', $documentValidators);
  
  if($file)
  {
  	/* 
  	 * If the document archive has been successfully uploaded:
  	 * - clean up its name and remove any weird characters
  	 * - copy it to the previously defined $dir directory
  	 * 
  	 */     
	  $filename=preg_replace("/[^a-zA-Z0-9\.\ ]/","_",$file->filename);  
	  $filename = home_directory_upload_rename($dir,$filename);
	  
	  $archiveFilename = $dir . $filename;
	  rename($file->destination, $archiveFilename);	  
  }
  else {
  	
  	/* 
  	 * If the document archive was not successfully uploaded, then
  	 * it is very likely it was provided as an URL.
  	 * 
  	 * - extract the filename from the URL
  	 * - clean up its name and remove any weird characters
  	 * - upload the URL to the previously defined $dir directory
  	 * 
  	 * fail with an error if upload did not succeed.
  	 */
      $filename = $form_values['values']['data_url'];
      if(strrpos($filename, "/")){
      	$filename = substr($filename, strrpos($filename, "/") + 1);
      }
      $filename = preg_replace("/[^a-zA-Z0-9\.\ ]/","_",$filename);

      	 
      $archiveFilename = $dir . $filename;
      
      $url = $form_values['values']['data_url'];

      if(!copy($url, $archiveFilename)){
	  	drupal_set_message('Upload Failed','error');
      	return;
      }
  }
  
  $new_page = false;
  $page_body = "";

  if($form_values['values']['description_field']['keywords']){
  	$new_page = true;
  	$page_body = "<strong>Keywords: </strong>" . $form_values['values']['description_field']['keywords'];
  }
  
  if($form_values['values']['description_field']['detailed_description']){
  	$new_page = true;
  	$page_body .= "\n<p><strong>Description</strong></p>";
  	$page_body .= $form_values['values']['description_field']['detailed_description'];
  }
  
  if($form_values['values']['description_field']['metadata']){
  	$new_page = true;
  	$page_body .= "\n<p><strong>Metadata</strong></p>";
  	$page_body .= $form_values['values']['description_field']['metadata'];
  }
  
  if(!$form_values['values']['interpretation_field']['new_interpretation']){
  	  	$new_page = true;
  	  	$page_body .= "\n<p><strong>Interpretation Information:</strong></p>\n";
  	  	if($form_values['values']['interpretation_field']['title']){
  	  		$page_body .= "<strong>Title: </strong>" . $form_values['values']['interpretation_field']['title'] . "\n";
  	  	}
    	if($form_values['values']['interpretation_field']['keywords']){
  	  		$page_body .= "<strong>Keywords: </strong>" . $form_values['values']['interpretation_field']['keywords'] . "\n";
  	  	}
    	if($form_values['values']['interpretation_field']['detailed_description']){
  			$page_body .= "\n<p><strong>Description</strong></p>";
  			$page_body .= $form_values['values']['interpretation_field']['detailed_description'];
 		}
  }
  
  if(!$form_values['values']['research_field']['new_research_task']){
    	$new_page = true;
    	$page_body .= "\n<p><strong>Research Tasks:</strong></p>\n";
    	if($form_values['values']['research_field']['title']){
  	  		$page_body .= "<strong>Title: </strong>" . $form_values['values']['research_field']['title'] . "\n";
  	  	}
    	if($form_values['values']['research_field']['detailed_description']){
  			$page_body .= "\n<p><strong>Description</strong></p>";
  			$page_body .= $form_values['values']['research_field']['detailed_description'];
 		}
  }
  
  if(!$form_values['values']['software_field']['new_software']){
    	$new_page = true;
    	
    	$page_body .= "\n<p><strong>Additional Software:</strong></p>\n";
      	if($form_values['values']['software_field']['detailed_description']){
  			$page_body .= "\n<p><strong>Description</strong></p>";
  			$page_body .= $form_values['values']['software_field']['detailed_description'];
 		}
      	if($form_values['values']['software_field']['technical_details']){
  			$page_body .= "\n<p><strong>Technical Details</strong></p>";
  			$page_body .= $form_values['values']['software_field']['detailed_description'];
 		}
  }
  
  if($form_values['values']['other_field']['related_publications']){
    	$new_page = true;
  		$page_body .= "\n<p><strong>Related Publications</strong></p>";
  		$page_body .= $form_values['values']['other_field']['related_publications'];
  }
  if($form_values['values']['other_field']['submitted_files']){
    	$new_page = true;
  		$page_body .= "\n<p><strong>Submitted Files</strong></p>";
  		$page_body .= $form_values['values']['other_field']['submitted_files'];
  }
  if($form_values['values']['copyright_field']['author']){
    	$new_page = true;
  		$page_body .= "\n<p><strong>Contact Author</strong></p>";
  		$page_body .= $form_values['values']['copyright_field']['author'];
  }
  if($form_values['values']['copyright_field']['own_copyright']){
    	$new_page = true;
  		$page_body .= "\n<p><strong>Additional Copyright Information</strong></p>";
  		$page_body .= $form_values['values']['copyright_field']['answered_no'];
  }
  
  if($new_page){
      $newnode = new stdClass();
      $newnode->title = $form_values['values']['data_dataset_name'];
      $newnode->type = "basic_page";
      $newnode->body = $page_body;
      $newnode->status = 1;
      $newnode = node_submit($newnode);
      node_save($newnode);
      $page_url = url("node/" . $newnode->nid);
      drupal_set_message('A page for the dataset has been created <a href="' . $page_url . '">here</a>.');
  }

  /*
   * Same treatment as for the document archive file upload, but this time for the 
   * Copyright file: either it is a regular file, and it will be uploaded using the form,
   * either it is an URL and will be fetched.
   */
  if($copyrightFile)
  {  
	  $filename=preg_replace("/[^a-zA-Z0-9\.\ ]/","_",$copyrightFile->filename);  
	  $filename = home_directory_upload_rename($dir,$filename);
	  
	  $copyrightFileName = $dir . $filename;
	  rename($copyrightFile->destination, $copyrightFileName);	  
  }
  else {
      $filename = $form_values['values']['data_copyright_url'];
      if(strrpos($filename, "/")){
      	$filename = substr($filename, strrpos($filename, "/") + 1);
      }
      $filename = preg_replace("/[^a-zA-Z0-9\.\ ]/","_",$filename);

      $copyrightFileName = $dir . $filename;
      
      $url = $form_values['values']['data_copyright_url'];
      copy($url, $copyrightFileName);
  }
  if($descriptionFile)
  {  
	  $filename=preg_replace("/[^a-zA-Z0-9\.\ ]/","_",$descriptionFile->filename);  
	  $filename = home_directory_upload_rename($dir,$filename);
	  
	  $descriptionFileName = $dir . $filename;
	  rename($descriptionFile->destination, $descriptionFileName);	  
  }
  
  /*
   * Phase 1:
   * Once all files have been uploaded, run the archive extraction script defined
   * in the Drupal configuration variable 'dae_data_preparse_script' 
   * (default value 'upload.pl')
   * 
   * This script extracts all files from the archive and generates an XML file
   * identifying all page_images and files, attaches copyright attributes and contributer
   * ids; structures sub-directories in datasets etc.
   * 
   * It creates most of the markup, but does not connect to the database to extract
   * existing type identifiers. Mapping identifiers is done in Phase 2.
   */
  $nameString = escapeshellarg($dir . $form_values['values']['data_dataset_name']);
  $descriptionString = escapeshellarg($form_values['values']['data_dataset_short']);
  $userId = getPID($user->uid);
  $command = escapeshellarg(variable_get('dae_data_preparse_script','upload.pl'));
  
  $commandString = "$command '$archiveFilename' $nameString $descriptionString $userId ". (isset($copyrightFileName) ? "'$copyrightFileName'" : '');
  $returnString = system($commandString . "> '$archiveFilename.log' 2>&1", $returnValue);
  
  if($returnValue) {
      shell_exec("rm '$archiveFilename' '$copyrightFileName' '$descriptionFileName'");
      //drupal_set_message($commandString,'error');
      drupal_set_message(t('Upload Error'),'error');
  }
  else {
      
  	/*
  	 * Phase 2:
  	 * 
  	 * Mapping the datatypes in the generated markup file.
  	 * 
  	 * Completes the markup file generated by 'upload.pl' by querying the database and by
  	 * retrieving the required ids for correct mapping.
  	 * 
  	 */
      $db = getDAEDatabase();
      
      $images = file_get_contents($dir . $form_values['values']['data_dataset_name'] . "_types.txt");
      
      $imageLabels = explode("\n",$images);
      $sedCommand = '';
      $imageIds = array();
      
      foreach($imageLabels as $label) {
          if($label != '') {
              $imageIds[$label] = $db->r('select ID from DATATYPE where NAME = :namelabel', array(':namelabel' => $label));
          }
      }

      $fIn = fopen($dir . $form_values['values']['data_dataset_name'] . "_partialdata.xml",'r');
      $fOut = fopen($dir . $form_values['values']['data_dataset_name'] . "_metadata.xml",'w');
      
      do {
          $line = fgets($fIn);
          fwrite($fOut,$line);
      } while($line && strpos($line, '<mapping>') === false);
      
      foreach($imageIds as $label => $id) {
          if($id) {
              fwrite($fOut,"
            <pair>
              <local>$label</local>
              <db>$id</db>
            </pair>");
          }
      }
      
      $line = fread($fIn,filesize($dir . $form_values['values']['data_dataset_name'] . "_partialdata.xml"));
      fwrite($fOut,$line);
      
      fclose($fIn);
      fclose($fOut);    
      
      //system("rm '$archiveFilename'");
      
      /*
       * Phase 3:
       * 
       * Uploading all extracted files to the final (read-only) destination for referencing by the database.
       * Parsing the markup file and generating all required entries in the database, as well as generating
       * thumbnails.
       * 
       */
      
      $command = escapeshellarg(variable_get('dae_data_upload_script','commit.sh'));
      $server = escapeshellarg(variable_get('dae_db_server','localhost'));
      $SID = escapeshellarg(variable_get('dae_db_name','DAE'));
      $mail = escapeshellarg($user->mail);
      
      $commandString = "$command $nameString $server $SID $mail > $nameString" . "_serverLog 2>&1 &";
      $returnString = system($commandString, $returnValue);
      
      if($returnValue) {
          shell_exec("rm '$archiveFilename' '$copyrightFileName' '$descriptionFileName'");
          shell_exec("yes | \rm -r $nameString $nameString". "_metadata.xml" . 
          	" $nameString". "_partialdata.xml" . 
          	" $nameString". "_types.txt" . 
          	" $nameString". "_workdir");
          drupal_set_message(t('Commit Error: '.$returnString),'error');
          return;
      }
      
      drupal_set_message(t('<p>First upload stage succeeded for dataset @datsetname.</p>
      		<p>Now switching to second stage: an e-mail notification will be sent upon completion.</p>
      		<p>You can now close this window or upload another dataset.</p>',array('@datsetname' => $form_values['values']['data_dataset_name'])));
      

  }
}

function dae_data_upload_page($form_state = NULL)
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
    
    $output .= drupal_get_form('dae_data_upload');
    
    return $output;
}

function dae_data_upload_validate($form_id, $form_values)
{
    $db = getDAEDatabase();
    
    $variables[':datasetname'] = $form_values['values']['data_dataset_name'];
    
    if($db->r('select count(*) from DATASET where lower(NAME) = lower(:datasetname)',$variables) != 0) {
        form_set_error('', t('Name \'@datasetname\' already in use', array('@datasetname' => $form_values['values']['data_dataset_name'])));
        return false;
    }
    
    return true;
}
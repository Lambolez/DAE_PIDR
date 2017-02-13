<?php 
/**
 * @file
 * 
 * @author  Mike Kot <mdk312@lehigh.edu>
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
 
 
function home_directory_run_batch()
{
	global $uploads, $user;
	
	drupal_set_title(t('My Uploads'));
	
	switch($_POST['operation'])
	{
		case 'download':
			$list = $_POST;
			unset($list['operation']);
			unset($list['moveToDest']);
			
			$list = home_directory_clean_filenames($list);

			$files = implode(" ",$list);
			
			$time = date("YmdHis");
			
			shell_exec("cd $uploads
			zip -r /dae/spool/".$user->uid."-$time.zip $files");// file like 1-20100519230721.zip is created
			
			drupal_goto("getFile/tmp/$time.zip");//only 20100519230721.zip is passed download function automatically adds user_id- to prevent access to any other files
			break;
			
		case 'delete':				
			require_once("./".drupal_get_path('module','home_directory')."/home_directory_uploads.inc");
			
			if(user_access('delete data'))
			{
				$list = $_POST;
				unset($list['operation']);
				unset($list['moveToDest']);
				
				foreach($list as $file):			
					if(home_directory_file_uploads_exists($file))
						home_directory_remove_file($file);
					else
						home_directory_remove_directory($file);
						
					drupal_set_message(t('!file deleted successfully.',array('!file' => home_directory_get_filename_from_path($file))),'status');
				
				endforeach;
				
				return home_directory_uploads(home_directory_get_path_to_file_dir($file));
			}
			else
			{
				drupal_set_message(t('You do not have permission to delete files'),'status');
				
				return home_directory_uploads();
			
			}
			break;
				
		case 'move':
			require_once("./".drupal_get_path('module','home_directory')."/home_directory_uploads.inc");
			
			$list = $_POST;
			unset($list['operation']);
			$dest = $list['moveToDest'];
			$destFormatted = str_replace(" ", "\ ", $dest);
			
			unset($list['moveToDest']);
			
			foreach($list as $file):
				if(file_exists($uploads . $file))
				{
					$name = home_directory_get_filename_from_path($file);
				
					if($file == $dest)
					{
						drupal_set_message(t('%dest cannot be moved into itself!',array('%dest' => "Uploads$dest")),'warning');
					}
					else if(file_exists("$uploads$destFormatted/$name"))
					{
						drupal_set_message(t('%file could not be moved because a file of the same name exists in %dest.',array('%file' => $name,'%dest' => "Uploads$dest")),'warning');	
					}
					else
					{
						shell_exec("mv \"$uploads$file\" \"$uploads$destFormatted/$name\"");
						drupal_set_message(t('%file was moved to %dest.',array('%file' => $name,'%dest' => "Uploads$dest")),'status');
					}
				}
			endforeach;
				
			return home_directory_uploads(home_directory_get_path_to_file_dir($file));
			
			break;
	
	}
}

function home_directory_clean_filenames($list)
{
	foreach($list as $item => $value):
		$list[$item] = substr($value,1);
		$list[$item] = str_replace(" ", "\ ", $list[$item]);
	endforeach;

	return $list;
}
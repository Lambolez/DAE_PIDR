<?php 

function home_directory_run_batch()
{
	global $uploads, $user;
	
	switch($_POST['operation'])
	{
		case 'download':
			$list = $_POST;
			unset($list['operation']);
			
			$list = home_directory_clean_filenames($list);

			$files = implode(" ",$list);
			
			$time = date("YmdHis");
			
			shell_exec("cd $uploads
			zip -r /dae/spool/".$user->uid."-$time.zip $files");// file like 1-20100519230721.zip is created
			
			drupal_goto("getFile/tmp/$time.zip");//only 20100519230721.zip is passed download function automatically adds user_id- to prevent access to any other files
			break;
			
		case 'delete':
			if(user_access('delete data'))
			{
				$list = $_POST;
				unset($list['operation']);
				unset($list['moveToDest']);
				$i=0;
				
				foreach($list as $file):			
					if(home_directory_file_uploads_exists($file))
						home_directory_remove_file($file);
					else
						home_directory_remove_directory($file);
					$i++;
				endforeach;
				
				$_SESSION['temp'] = $i . " files";
				
				drupal_goto('uploads/deleted/');
			}
			else
				drupal_goto('uploads/not-deleted/');
			break;
				
		case 'move':
			
			$list = $_POST;
			unset($list['operation']);
			$dest = $list['moveToDest'];
			$dest = str_replace(" ", "\ ", $dest);
			
			unset($list['moveToDest']);
			
			foreach($list as $file):
			
				if($file == $dest)
				{
					drupal_goto("uploads/self-move$dest");
					return;
				}
			
			endforeach;
			$i=0;
			foreach($list as $file):
				if(file_exists($uploads . $file))
					shell_exec("mv \"$uploads$file\" \"$uploads$dest/$name\"");
				$i++;
			endforeach;
			
			$_SESSION['temp'] = $i . " files";
			
			drupal_goto('uploads/moved/');
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
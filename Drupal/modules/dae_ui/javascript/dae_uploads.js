function selectAllUploads()
{
	 $('input:checkbox').attr('checked', true);
}
function selectUploads(theClass)
{
	 selectNoneUploads();
	 $(theClass).attr('checked', true);
}
function selectNoneUploads()
{
	 $('input:checkbox').attr('checked',false);
}
function setBatchOp(op)
{
	$('.batchOp:hidden').val(op);
	
	if(op == "delete")	
	{
		if(!checkChecked())
			return;
		if($(".dirBox:checked").length == 0)
			jConfirm('Are you sure you want to delete these files?', 'Delete Files?', function(r) 
			{
				if(r)
					form = document.file_list.submit();
			});
		else if($(".dirBox:checked").length > 0)
			jConfirm('Are you sure you want to delete the selected directories and all files or subdirectories they may contain?', 'Delete Files?', function(r) 
			{
				if(r)
					form = document.file_list.submit();
			});
	}
	else if(op == "move")
	{
		$('#hd-move_to_options').slideToggle("fast");
	}	
	else if(op == "download")
	{
		if(!checkChecked())
			return;
		form = document.file_list.submit();
	}
}
function batchMoveTo(name,dest)
{
	if(!checkChecked())
		return;
		
	$('.moveToDest:hidden').val(dest);
	
	if($(".dirBox:checked").length == 0)
		jConfirm('Are you sure you want to move these files to '+name+'?', 'Move Files?', function(r) 
		{
			if(r)
				form = document.file_list.submit();
		});
	else if($(".dirBox:checked").length > 0)
		jConfirm('Are you sure you want to move the selected directories and all files or subdirectories they may contain to '+name+'?', 'Move Files?', function(r) 
		{
			if(r)
				form = document.file_list.submit();
		});
	
}
function checkChecked()
{
	if($(":checkbox:checked").length == 0)
	{
		jAlert('No files are selected.','Warning');
		return false;
	}
	else
		return true;
}
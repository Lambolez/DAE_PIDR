function moveFile(select)
{
	
	if(!select.options[select.selectedIndex] == "")
		window.location = select.options[select.selectedIndex].value;

}

function show_desc(moreInfo)
{
	  $(moreInfo).slideToggle();
}

function newSubDirectory()
{
	var newFolder = "<form action=\"#\"><div class=\"hd-upload_row\"><span class=\"left\"><img src=\"modules/home_directory/images/folder.png\" width=\"24\" height=\"24\" alt=\"\" /><input type=\"text\"></span><img class=\"delete\" src=\"modules/home_directory/images/cancel.png\" alt=\"\" /></div></form>";
					
	$('#directories').append(newFolder);
}
function showRunOutput(run_id)
{
	var statusDiv;
	statusDiv = $('#run_status-'+run_id);
	
	if($('#output-'+run_id).css("display") == "none")
		$('#output-'+run_id).load("?q=getRunInfo/"+run_id);
	
	if(statusDiv.length)
	{
		$.get("?q=getRunInfo/done/"+run_id, function(data) {
	  		if(data == "true")
	  			updateComplete(statusDiv);
	  			
	  	});
	}
	
	$('#output-'+run_id).slideToggle();
	
}

function updateComplete(statusDiv)
{
	var status;
	status = statusDiv.html();
	status = status.replace("In Progress","Complete");
	status = status.replace("running.gif","check.png");
	statusDiv.removeClass("algorithm_run_running");
	statusDiv.addClass("algorithm_run_complete");
	statusDiv.html(status);
}
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
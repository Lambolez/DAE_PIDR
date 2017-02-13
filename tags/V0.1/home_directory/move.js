function moveFile(select)
{
	
	if(!select.options[select.selectedIndex] == "")
		window.location = select.options[select.selectedIndex].value;

}

function show_desc(li)
{
	  $(li).children().slideDown();
}

function newSubDirectory()
{
	var newFolder = "<form action=\"#\"><div class=\"hd-upload_row\"><span class=\"left\"><img src=\"modules/home_directory/images/folder.png\" width=\"24\" height=\"24\" alt=\"\" /><input type=\"text\"></span><img class=\"delete\" src=\"modules/home_directory/images/cancel.png\" alt=\"\" /></div></form>";
					
	$('#directories').append(newFolder);
}
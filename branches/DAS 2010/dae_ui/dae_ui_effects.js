var subSets;

function showListInfo(div)
{
	$(div).next().slideToggle();
}
function showSubSets(div,parent)
{
	var listItem = $(div).parent();
	var subsetContainer = $(div).next();

	if(subsetContainer.html() == parent)
	{
		
		$('#loading-'+parent).html("<img src=\"sites/default/modules/home_directory/images/ajax.gif\" alt=\"\">");
		$('#loading-'+parent).fadeIn();
		subsetContainer.load("?q=browse/subset/"+parent,function() {
 		 	subsetContainer.slideToggle();
 		 	listItem.toggleClass('expand');
			listItem.toggleClass('collap');
			$('#loading-'+parent).fadeOut();
		});
		
	}
	else	
		subsetContainer.slideToggle(
		function()
		{
			listItem.toggleClass('expand');
			listItem.toggleClass('collap');	
		}
		);
}
function checkChildren(box)
{
	 var contains = $(box).parent().parent();
	
	 if($(box).attr('checked') == true)
	 {
		 $(contains).find('input:checkbox').attr('checked',true);
	 }
	 else
	 {
	 	$(contains).find('input:checkbox').attr('checked',false);
	 	$('#dae-data_list').find('input:checkbox').not(contains.find('input:checkbox')).attr('checked',false);
	 }
}
function show()
{
	return;
}
function showRunOutput(div,runID)
{	
	var listItem = $(div).parent().parent();
	var infoContainer = $(div).next(".dae-run-info");

	if($('#loading-'+runID).html() != "<img src=\"sites/default/modules/home_directory/images/ajax.gif\" alt=\"\">")
	{
		
		$('#loading-'+runID).html("<img src=\"sites/default/modules/home_directory/images/ajax.gif\" alt=\"\">");
		$('#loading-'+runID).fadeIn();
		var clear = infoContainer.html();
		infoContainer.load("?q=getRunInfo/"+runID,function() {
			infoContainer.append(clear);
 		 	infoContainer.slideToggle();
 		 	listItem.toggleClass('expand');
			listItem.toggleClass('collap');
			$('#loading-'+runID).fadeOut();
		});
		
	}	
	else	
		infoContainer.slideToggle(
		function()
		{
			listItem.toggleClass('expand');
			listItem.toggleClass('collap');	
		}
		);
	
}
function showFileInfo(div)
{
	var listItem = $(div).parent().parent();
	$(div).next().slideToggle(
		function()
		{
			listItem.toggleClass('expand');
			listItem.toggleClass('collap');	
		});

}
function flagDataSet(dataset)
{
	jPrompt('Describe Problem:', '', 'Flag Dataset', function(r) {
	    if(r)
	    {
	    	document.flagForm.ds.value =dataset;
	    	document.flagForm.comments.value = r;
	    	document.flagForm.submit();
	    }
	});
}
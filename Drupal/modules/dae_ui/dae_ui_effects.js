/**
 * @file dae_ui_effects.js
 * 
 * @author  Mike Kot <mdk312@lehigh.edu>
 * 
 * @version 1.0
 *
 * \brief General javascript functions that are always included
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
 * 
 */

//this behavior is used to make the execution graph appear when necessary through the use of an invisible anchor
Drupal.behaviors.infovis = function (context) {
	$(".infovis_anchor").bind('click', function() {
  	window.location.href = this.href;
  	return false;
	});

	
    	$('.infovis_anchor').trigger('click');
	$('.infovis_anchor').removeClass('infovis_anchor');

};



var subSets;
var listitemname;
var boldStart;
var zoom = 5;
var SearchRegExp;
var pageImageWidth, pageImageHeight;
var ajaxPath = "sites/default/modules/dae_ui/images/ajax.gif";

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
		
		$('#loading-'+parent).html("<img src=\""+ajaxPath+"\" alt=\"\">");
		$('#loading-'+parent).fadeIn();
		subsetContainer.load("?q=browse/subset/"+parent,function() {
 		 	subsetContainer.slideToggle();
 		 	listItem.toggleClass('expand');
			listItem.toggleClass('collap');
			$('#loading-'+parent).fadeOut();
		});
		
	}
	else	
		subsetContainer.slideToggle(function()
		{
			listItem.toggleClass('expand');
			listItem.toggleClass('collap');	
		});

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

	if($('#loading-'+runID).html() != "<img src=\""+ajaxPath+"\" alt=\"\">")
	{
		
		$('#loading-'+runID).html("<img src=\""+ajaxPath+"\" alt=\"\">");
		$('#loading-'+runID).fadeIn();
		var clear = infoContainer.html();
		infoContainer.load("?q=run/info/"+runID,function() {
			infoContainer.append(clear);
 		 	infoContainer.slideToggle();
 		 	listItem.toggleClass('expand');
			listItem.toggleClass('collap');
			$('#loading-'+runID).fadeOut();
			Drupal.behaviors.infovis();
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
function runDelete(run)
{
	jConfirm('Are you sure you want to delete this run?', 'Delete Run?', function(r) 
			{
				if(r)
				{
					var runli = $("#run-"+run);
					
					runli.children(":first").html("<span class=\"deleting-run\"> Deleting...</span>");
					
					$.get("?q=run/delete/"+run,
						function()
						{
							runli.slideUp();
						}
					);
					
				}
			}
	);
}
$(document).ready(function()
{
	$(".star").mouseover(function() 
	{
		var starNum;
		ratingDiv = $(this).parent().parent();
		filled = ratingDiv.find('.my_rating,.general_rating');
		filled.css("background","url('sites/default/modules/dae_ui/images/stars.png') 0 -60px no-repeat");
		starNum = this.id.substr(5);
		
		filled.css("width",starWidth*starNum+"px");

	});
	$(".rating").mouseout(function() 
	{
		if(filled.attr("rel").substr(0,1) == "0")
			filled.css("background","url('sites/default/modules/dae_ui/images/stars.png') 0 0 no-repeat");
		curRating = filled.attr("rel").substr(2);
		filled.css("width",curRating+"px");
	});
	
	$(".star").click(function() 
	{
		var rating = this.id.substr(5);
		filled.css("background","url('sites/default/modules/dae_ui/images/stars.png') 0 -30px no-repeat");
		var star = this;
		var dataset = ratingDiv.attr("id").substr(5);
		var width = starWidth*rating;
		
		$.get("?q=browse/dataitem/rate/"+ dataset +"/"+rating,
			function(transport)
			{	
					filled.css("background","url('sites/default/modules/dae_ui/images/stars.png') 0 -60px no-repeat");
					filled.css("width",width+"px");
					filled.attr("rel","1-"+width);
			}
		);
	});
	
	//added by yil308 for dropdown list.
	var obj = window.document.getElementById('dae_data_search_dropdown');

	$("#edit-dae-data-search").focus(function(){if($(this).val() == "Search Names and Tags") $(this).val('');})
	
	
	if($("#dae_data_search_dropdown").length)//added my mdk312 for non search bar pages
		obj.onchange=
		function(){
				//alert(obj.options);
				var value = obj.options[obj.options.selectedIndex].text;
				window.document.getElementById('edit-dae-data-search').value=value;
				//$("#edit-dae-data-search").focus();
				value = value.substring(0, value.indexOf(' '));
				//alert(value);
				$.get(Drupal.settings.basePath+"?q=browse/search/select/" + value, function(data){
				//alert(data);
					var myObject = eval('(' + data + ')');
					//alert(myObject[0].id);
					window.location = Drupal.settings.basePath+"?q=browse/"+myObject[0].id;
				});
		};
	
	
//	$("#edit-dae-data-search").autocomplete({
//			source: Drupal.settings.basePath+"?q=browse/search/",
//			minLength: 2,
//			select: function(event, ui) {
//				window.location = Drupal.settings.basePath+"?q=browse/"+ui.item.id;
//			}
//		});
	$.widget("custom.catcomplete", $.ui.autocomplete, {
		_renderMenu: function( ul, items ) {
			var self = this,
				currentCategory = "";
			$.each( items, function( index, item ) {
				if ( item.category != currentCategory ) {
					ul.append( "<li class='ui-autocomplete-category' role='menuitem'>" + item.category + "</li>" );
					currentCategory = item.category;
				}
				self._renderItem( ul, item );
			});
		}
	});
	$('#edit-dae-data-search').catcomplete({
			delay: 0,
			source:  Drupal.settings.basePath+"?q=browse/search/",
			minLength: 2,
			select: function(event, ui) {
				window.location = Drupal.settings.basePath+"?q=browse/"+ui.item.id;
			}
		});
		
		
	$("body").append("<div class=\"popup-close\"></div>");
	$("#dae-data-browse-popular .active").click(function(){
		$(".popup-close").css("display","block");
		$("#dae-data-popular-options").css("display","block"); 
		$(".popup-close").click(function(){ $(".popup-close").css("display","none");  $(".popup").css("display","none"); });
		return false;
	});
	$(".dae-data-browse-list-item-large h3 a").each(function(){
		if(search.length > 0)
		{
			listitemname = $(this).html();
			searchRegExp = new RegExp(search,"i");
			boldStart = listitemname.search(searchRegExp);
			if(boldStart >= 0)
				$(this).html( listitemname.substr(0,boldStart) + "<strong>" + listitemname.substr(boldStart,search.length) + "</strong>" + listitemname.substr(boldStart+search.length));
		}
	});
	Drupal.attachBehaviors();
});

$(document).ajaxError(function(e, xhr, settings, exception) {
  	forbiddenMessage(xhr.status);
});

function forbiddenMessage(httpStatus)
{
	if(httpStatus == 403){
		jAlert('You must <a href="?q=user/login">login</a> to use this feature.','Warning');
		return false;
	}
	return true;
}
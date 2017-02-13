var starWidth = 18;
var filled;
var ratingDiv;
var selectedItem = 0;
var pageimage = false;
var scrollLoad = false;
var listScrollLoc;
var infoWidth;
var on;
var rci,tci;

/*

	On ready functions such as on clicks and text-shortener

*/
$(window).ready(function(){if($("#page-image").length){$("#page-image").pageImageResize({maxWidth: $(".page-image-wrapper").width()-23, elements: false});}});
$('body').ready(function(){if($("#page-image").length){$("#page-image").attr("src",Drupal.settings.basePath+"?q=browse/dataitem/thumb/"+selectedItem+"/large");  $("#page-image").pageImageResize({maxWidth: $(".page-image-wrapper").width()-23, elements: false});}});
$(document).ready(function()
{
	pageImageClick();
	
	$(".dataset-li").click(function()
	{datasetClick(this);});
	toolbarClick();
	fixList();
	hashedDataitemLoad();
	activateTools();
	pageElementPropertyValueAccordion();
	pageElementInterpretationHandler();
	pageElementInterpretFormExpand();
	pageElementTypeHandler();
	pageElementTypeAutoComplete()
	
	$('input[name=howPEType]').change(function(){
	
		pageElementTypeHandler();
		
	});
	
	$("body").append("<div class=\"popup-close\"></div>");
	
	
	if($("#data-item-page-element").length) //page element page
	{
		var ar = mask[2]/mask[3];
		var mWidth, mHeight;
		
		if(mask[3] > $(".page-image-wrapper").width()-15)
		{
			mWidth = $(".page-image-wrapper").width()-15;
			mHeight = mWidth*ar;
		}
		else
		{
			mWidth = mask[3];
			mHeight = mask[2];
		}
			
		var rx = mWidth/mask[3];
		var ry = mHeight/mask[2];
		
		$("#page-image-mask").css({
			width: mWidth + 'px',
			height: mHeight + 'px',		
		});
		$("#data-item-page-element").css({
			position: 'absolute',
			top: "-" + mask[1]*ry + 'px',
			left: "-" + mask[0]*rx + 'px',
			width: truesize[0]*rx + 'px',
			height: truesize[1]*ry + 'px',
		});
		
	}
	else if($("#page-image").length) //page image page
	{
		$("#page-image").pageImageResize({maxWidth: $(".page-image-wrapper").width()-23, elements: false});
	}	
	
	$(".page-image-wrapper").hover(function(){ 
			if($("#page-image-mask").height() + 5 > $(".page-image-wrapper").height())
				$(".expand-page-image").fadeIn(); 
		},function(){ 
			if($("#page-image-mask").height() + 5 > $(".page-image-wrapper").height())
				$(".expand-page-image").fadeOut();
		}
	);
	$(".expand-page-image").click(function(){ 
		var height = $("#page-image-mask").height() + 5;
		$(".expand-page-image").fadeOut(); 
		$(".page-image-wrapper").css("height",height + 'px');  
		$(".page-image-container").css("height",(height+10) + 'px'); 
		return false;
	});
	
	$("#edit-dae-data-tag").autocomplete({
			source: Drupal.settings.basePath+"?q=browse/tags/search/",
			minLength: 1,
			select: function(event, ui) {
				$("#edit-dae-data-tag-id").val(ui.item.id);
			}
		});
			
	$(".dataset-li").dblclick(function()
	{
		window.location = Drupal.settings.basePath+"?q=browse/dataitem/"+ this.id.substr(8);
		
	});
	
	$(".dataset-list-selected").dblclick(function()
	{
		window.location = Drupal.settings.basePath+"?q=browse/dataitem/"+ this.id.substr(9);
		
	});
	
	$(".comment-body").each(function()
	{
		var content;
		content = $(this).html();
		$(this).html(content.replace(/\n/g,"<br/>"));
	});
	
	
	$("#edit-dae-data-comment").focus(function()
	{
		if($("#edit-dae-data-comment").val() == "Comment on this data")
			$("#edit-dae-data-comment").val("");
	});
	$("#dataset-search-field").focus(function()
	{
		if($("#dataset-search-field").val() == "Search")
			$("#dataset-search-field").val("");
	});
	
	$(".dataset-list").resizable({ handles: 'e',minWidth: 250, resize: function(event, ui) { 
		
		fixList();
		fixMeta(); 
		}  
	});
	
	if(location.hash == '#show-elements'){
		$("#dae-toolbar-view-elements").click();
	}
	
	
	$(".dataset-contents-wrapper").resizable({alsoResize: '.dataset-list,#dataset-item-info', handles: 's'});
	$(".page-image-container").resizable({alsoResize:'.page-image-wrapper',minHeight:100, handles: 's'});
	$(".dataset-list").scroll(function(){ var hold; var dataitem; var list; 
		if($(this).scrollTop() > listScrollLoc)
		$(".load-more").each(function()
		{
			dataitem = this.id.substr(17);
			page = parseInt(this.title) + 1;
			list = $(this).parent();
			$(this).addClass("loading-more");
			$(this).removeClass("load-more");
			hold = $(this).parent().html();
			$.get(Drupal.settings.basePath+"?q=browse/dataitem/list/"+dataitem+"/"+page, function(data) {
	  			list.html(hold + data);
	  			$(".loading-more").remove();
	  			pageImageClick();
	  			fixList();
			});
		});
		listScrollLoc = $(this).scrollTop();
	});
		
});
function activateTools()
{
	$(".dataset-toolbar-item").removeClass("inactive");
	
	if(selectedItem == 0)
		$(".dataset-toolbar-item").addClass("inactive");
	else if(!pageimage)
		$(".pageimage-op").addClass("inactive");
}
/**
 * Autoloads dataitem
 *
 */
function hashedDataitemLoad()
{
	var loadClick = $("#dataitem-"+location.hash.substr(1));
	if(loadClick.size())
		loadClick.click();
}
/**
 * Handles clicks on toolbar items
 *
 * When the user tries to download and copy, there is copyright information on the message box. 
 * The user can click that copyright name to see the detail of it.
 */
function toolbarClick()
{
$(".dataset-toolbar li:not(.inactive)").click(function(){
		if($(this).hasClass('inactive'))
			return false;
		switch(this.title)
		{
		case 'Download':
			$.get(Drupal.settings.basePath+"?q=browse/dataitem/beforedownload/" + selectedItem, function(data) {//
			if (data != "") {
				jConfirm('The selected pages have following copyrights (' + data + '). By clicking OK you are certifying that you have read the copyright notice(s). ' , 'Download Files', function(r)
						{
							if(r)
								window.location = Drupal.settings.basePath+"?q=browse/dataitem/download/" + selectedItem;
						});
				}
			else {
				window.location = Drupal.settings.basePath+"?q=browse/dataitem/download/" + selectedItem;
			}
				});
			
			break;
		case 'Copy':
			$.get(Drupal.settings.basePath+"?q=browse/dataitem/beforedownload/"+ selectedItem, function(data) {
			if (data != "") {
				jConfirm('The selected pages have following copyrights (' + data + '). By clicking OK you are certifying that you have read the copyright notice(s). ' , 'Copy Files', function(r)
						{
							if(r)
								window.location = Drupal.settings.basePath+"?q=browse/dataitem/copy/" + selectedItem;
						});
				}
			else {
				window.location = Drupal.settings.basePath+"?q=browse/dataitem/copy/" + selectedItem;
			}
			});
			break;
			
		case 'Run':
			if(!pageimage)
			{
				jAlert('Please select a single file.','Warning');
				return;
			}
			if(rci != selectedItem)
			{
				$("#possible-algorithms").html("<img src=\""+Drupal.settings.basePath+"sites/default/modules/dae_ui/images/ajax.gif\" alt=\"\" />");
				loadRunOpts();
				rci = selectedItem;
			}
			$(".popup-close").css("display","block");
			$("#run-menu").css(
			{
				left: $("#dae-toolbar-run").position().left-12,
				display: "block"
			});
			$("#possible-algorithms").click(function(){event.stopPropagation();});
			$(".popup-close").click(function(){$(".popup-close").css("display","none"); $("#run-menu").css("display","none");});

			break;
		case 'Tag':
		
		if(pageimage)
			$("#name-"+selectedItem).html()
			on = $("#name-"+selectedItem).attr("title");
			$("#dae-data-tag-on").html(on);
			$("#edit-dae-data-tag-on").val(selectedItem);
			$("#edit-dae-data-tag").keypress(function(){$("#edit-dae-data-tag-id").val("");});
			$(".popup-close").css("display","block");
			$("#tag-menu").css(
			{
				left: $("#dae-toolbar-tag").position().left-12,
				display:"block"
			});
			$("#tag-form").click(function(){event.stopPropagation();});
			$(".popup-close").click(function(){$(".popup-close").css("display","none"); $("#tag-menu").css("display","none");$("#edit-dae-data-tag-id").val("");});
			$("#edit-dae-data-tag").focus();
			break;
		case 'Tree':
			if(tci != selectedItem)
			{
				$("#tree-list").html("<img src=\""+Drupal.settings.basePath+"sites/default/modules/dae_ui/images/ajax.gif\" alt=\"\" />");
				loadTree();
				tci = selectedItem;
			}		
			$(".popup-close").css("display","block");
			$("#tree-menu").css(
			{
				left: $("#dae-toolbar-tree").position().left-12,
				display: "block"
			});
			$("#tree-list").click(function(){event.stopPropagation();});
			$(".popup-close").click(function(){$(".popup-close").css("display","none");$("#tree-menu").css("display","none"); });
			break;
		case 'Add Element':
			addPageElementClick();
			break;
		case 'View Elements':
			viewPageElementsClick();
			break;
		}
	});
}
function loadRunOpts()
{
	$("#possible-algorithms").load(Drupal.settings.basePath+"?q=browse/dataitem/runoptions/"+ selectedItem,function(){
					
		$("#possible-algorithms a").each(function()
		{				
			$(this).attr("href",$(this).attr("href")+ selectedItem + "/as/" + $(this).attr("rel"));
		});
		
	});
}
function loadTree()
{
	$.getJSON(Drupal.settings.basePath+"?q=browse/tree/"+selectedItem, function(data) 
		{
  			var containedby = data.cb;
  			var contains = data.cs;
  			var i =0, j=0;
  			
  			var output ="";
  			if(containedby && containedby.length)
  			{
  				output = "<span class=\"drop-down-label\">Contained By:</span>";
  			
  				if(containedby[i].mask)
  				{
  					output = output + "<a href=\""+Drupal.settings.basePath+"?q=browse/dataitem/"+containedby[i].id+"\"><div id=\"tree-page-image-parent\" style='position:relative;'><img src=\"" + containedby[i].thumb.path + "\" alt=\"\" width=\""+containedby[i].thumb.width+"\" height=\""+containedby[i].thumb.height+"\" /><div class=\"page-element-tree-mask\" style='position:absolute;top:"+(parseInt(containedby[i].mask['top'])+8)+"px;left:"+(parseInt(containedby[i].mask['left'])+10)+"px;width:"+containedby[i].mask['width']+"px;height:"+containedby[i].mask['height']+"px;'></div></div>" + containedby[i].name + "</a>";
  					
  					i++;
  				}
  				else
		  			while(i < containedby.length)
		  			{
		  				output = output + '<li><a href=\"'+Drupal.settings.basePath+'?q=browse/dataitem/'+containedby[i].id+'\">' + containedby[i].name + '</a></li>';
		  				i++;
		  			}
  			}
  			
  			if(contains && contains.length)
  			{
  				output = output + "<span class=\"drop-down-label\">Contains:</span>";
	  			
	  			while(i < contains.length)
	  			{
	  				output = output + '<li><a href=\"'+Drupal.settings.basePath+'?q=browse/dataitem/'+contains[i].id+'\">' + contains[i].name + '</a></li>';
	  				j++;
	  			}
  			}
  			if(i == 0 && j == 0)
  				output = "No Associations";
  			$('#tree-list').html(output);
	  			
	  });
}
/**
 * Handles clicks on expandable dataset folders
 *
 */
function datasetClick(it)
{
		$(".dataset-li,.data-item-li").removeClass("dataset-list-selected");
		$(it).addClass("dataset-list-selected");
		pageimage = false;
		var dataset = it.id.substr(8);
		selectedItem = dataset;
		activateTools();
		var children = $(it).next('.dataset-pad');
		if(!$(it).hasClass('already-loaded'))
		{
			var folderIcon = $(it).children("span");
			folderIcon.addClass("dataitem-loading");
			children.load(Drupal.settings.basePath+"?q=browse/dataitem/list/"+ dataset, function(){
				children.find(".dataset-li").click(function(){datasetClick(this);});
				folderIcon.removeClass("dataitem-loading");
				children.slideToggle(function(){ $(".dataset-list .ui-resizable-e").height($('.dataset-list li:visible').size()*22);});
				pageImageClick();
				fixList();
				$(".data-item-li").dblclick(function()
				{
					window.location = Drupal.settings.basePath+"?q=browse/dataitem/"+ it.id.substr(9);
				});
			});
			$(it).addClass('already-loaded');
		}
		else
		{
			children.slideToggle();
			fixList();
		}
}
/**
 * Handles clicks on page images
 *
 */
function pageImageClick()
{
$(".data-item-li").not("#dataitem-0,.dataset-list-selected").click(function() 
	{
		var dataitem;
		pageimage = true;
		$("#dataset-item-info").html("<div class=\"data-item-loader\"><img src=\""+Drupal.settings.basePath+"sites/default/modules/dae_ui/images/ajax.gif\" alt=\"\" /></div>");
		$(".dataset-li,.data-item-li").removeClass("dataset-list-selected");
		$(this).addClass("dataset-list-selected");
		dataitem = this.id.substr(9);
		location.hash = dataitem;
		var thumb = $('<img />').attr('src', Drupal.settings.basePath+'?q=browse/dataitem/thumb/'+dataitem+'/medium');
		selectedItem = dataitem;
		activateTools();
		$("#dataset-item-info").load(Drupal.settings.basePath+"?q=browse/dataitem/dataitem/"+dataitem,function(){
			fixMeta();
			$(this).find(".data-item-info-block:first a").html(thumb);
			$("#dataset-item-info").fadeIn(); 
		});
		
	});
}
/**
 * Resizes data item info text (right panel)
 *
 */
function fixMeta()
{
	infoWidth = $("#dataset-item-info").width() - $(".dataset-list").width();
	infoWidth = Math.floor(infoWidth/10);
	nameShortener(infoWidth-21,4); 
}
/**
 * Resizes dataset and page image text (left panel)
 *
 */
function fixList()
{
	$(".data-item-li span,.dataset-li span").textShorten({container: 'parent',right: 6, title: true, characterWidth: 7});
/*
	$(".data-item-li span,.dataset-li span").each(function()
	{
		listSize = Math.floor(($(this).parent().width())/7);
		$(this).textShorten({maxCharacters: listSize, right: 6, title: true});
		
	});
*/
}
function nameShortener(max,right)
{
	$(".dataitem-meta li span").textShorten({maxCharacters: max, right: right, title: true});
}

function helpful(ans,cid)
{
	$("#c-rate-loader-"+cid).css("display","inline");
	
	$.get(Drupal.settings.basePath+"?q=browse/dataitem/comment/" + cid + "/" + ans,
			function()
			{			
				setTimeout($("#c-rate-loader-"+cid).parent().html("Saved"),1000);
			}
		);
}

/**
 *	textShorten jQuery Plugin
 *  
 *  Takes the html body of all matching selected elements and ensures that
 *  it is no more than the number of characters specified in maxCharacters
 *
 *  @param options - options array which includes
 *					maxCharacters - most characters allowed in the string
 *					right - number of characters on the right side of the string to perserve
 *					title - use the element's title attribute instead of html.  This is useful
 *							when the text has already been shortened and the original string was
 *							lost
 *					container - a jQuery object that the string must fit inside the width of
 *								optionally the string 'parent' to use each element's parent
 *					characterWidth - the average pixel width of a character in the string
 */

$.fn.textShorten = function(options) {
  settings = $.extend({
     maxCharacters: 15,
     right: 5,
     title: false,
     container: false,
     characterWidth: 8
  }, options);
 	
 return this.each(function(){
  	var left, right, max;
 	
 	if(!settings.title)
 		content = $(this).html();
 	else
 		content = this.title;
 		
 	if(!settings.container)
 		max = settings.maxCharacters;
 	else if(settings.container == 'parent')
 		max = Math.floor($(this).parent().width()/settings.characterWidth);
 	else
 	{
 		max = Math.floor(settings.container.width()/settings.characterWidth);
 		
 	}
 	if(content.length > max)
	{	
		left = content.substr(0,max-settings.right-3);
		right = content.substr(content.length-settings.right);
		
		content = left + "..." + right;
	}
	$(this).html(content); 
 });

}
$(window).resize(function() {
	fixList();
	fixMeta();
});

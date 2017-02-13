var addPageElements = false;
var jCrop;
var cursize = new Array();
var peCoords = new Array();
var elementsready = false;
$(window).resize(function() {
	 if($("#page-image").length)
	 	if(typeof(page_elements) == 'undefined')
		 $("#page-image").pageImageResize({elements: false});
		else
		 $("#page-image").pageImageResize();
});


/**
 * Page element property value accordion
 *
 *
 */
function pageElementPropertyValueAccordion(){

	$("#page-element-property-value-accordion").accordion({
			autoHeight: false,
			navigation: true,
			collapsible: true,
			active: false
	});
	
	$('input[name=how]').change(function(){
	
		pageElementInterpretationHandler();
		
	});
	
	$("#edit-typeA option").each(function(){
		if($(this).val() == 'null')
			$(this).attr("disabled","disabled");
	});
}
/**
 *
 * Page element interpretation handler
 *
 *
 */
function pageElementInterpretationHandler(){
 
 	var method = $('input[name=how]:checked').val();
 	
 	if(method == 1){
 		$("#edit-typeA-wrapper").css("display","block");
 		$("#edit-typeB-wrapper").css("display","none");
 	}else{
		$("#edit-typeB-wrapper").css("display","block");
 		$("#edit-typeA-wrapper").css("display","none");
 	}
 
}
function pageElementTypeHandler(){
 
 	var method = $('input[name=howPEType]:checked').val();
 	
 	if(method == 1){
 		$("#edit-PEtypeA-wrapper").css("display","block");
 		$("#edit-PEtypeB-wrapper").css("display","none");
 	}else{
		$("#edit-PEtypeB-wrapper").css("display","block");
 		$("#edit-PEtypeA-wrapper").css("display","none");
 	}
}
function pageElementTypeAutoComplete(){

	$("#edit-PEtypeA option").each(function(){
		if($(this).val() == 'null')
			$(this).attr("disabled","disabled");
	});


/*
	$("#edit-PEtypeB").autocomplete({
		source: Drupal.settings.basePath+"?q=browse/search/pageelement/types",
		minLength: 2,
		select: function( event, ui ) {
			if(ui.item){
				$("#edit-PEtypeBMatchingID").val(ui.item.id);
			}
		}
	});
*/
}


function pageElementInterpretFormExpand(){

	$("#edit-dae-data-page-element-submit").click(function(){
	
		$("#page-element-toolbar").animate({height: "auto"},500);
		$("#page-element-info-form").slideDown();
		$("#edit-dae-data-page-element-submit").css("display","none");
		
		return false;
	
	});

}

/**
 *
 * Viewing Page Elements
 *
 * viewPageElementsClick   - event handler for a click on the view elements button on the toolbar
 * jQuery.pageImageResize  - resize the page image based on current browser conditions
 * jQuery.drawPageElements - redraw page elements
 *
 *
 *
 *
 *
 */

function viewPageElementsClick(){

	if($("#page-element-list").css("display") == "none")
	{
		if(typeof(page_elements) == 'undefined')
		{
			$(".dataset-toolbar li").addClass('inactive');
			var wrap = $('.page-image-wrapper').offset();
			var wrapper = $('.page-image-wrapper');
			
			$('body').append("<div id='page-element-hide-loader' style='position:absolute;top:"+ wrap.top +"px;left:"+ wrap.left +"px;width:"+ wrapper.width()+ "px;height:"+wrapper.height() +"px;background:#ffffff;opacity:.95'><div style='margin:"+((wrapper.height()/2)-16)+"px 0 0 "+((wrapper.width()/2)-100)+"px'><img  src='"+Drupal.settings.basePath+"/sites/default/modules/dae_ui/images/pe-loader.gif' alt='Loading...'/></div></div>");
			
			$.getJSON(Drupal.settings.basePath+"?q=browse/dataitem/load/pixel-mask/"+selectedItem, function(data) 
			{
				$('#page-element-hide-loader').fadeOut(function(){$('#page-element-hide-loader').remove();});
				page_elements = data;
				$("#page-image").pageImageResize();
				pageElementControlsActivate();
				$(".dataset-toolbar li").removeClass('inactive');
				displayPageElements();
				elementsready = true;
				reinitjCrop();
			});
		}
		else
		{
			displayPageElements();
		}
			
	}
	else
	{
		$(".page-element").fadeOut();
		$(".page-image-wrapper").css({padding:0});
		$("#page-image").pageImageResize();
		$("#page-element-list").animate({right: '-'+$("#page-element-list").width()+'px'},500,function(){$("#page-element-list").css("display","none");});
	}
	if(addPageElements && elementsready){
		reinitjCrop();
	}

}

function reinitjCrop(){
	jCrop.destroy();
	jCrop = $.Jcrop($("#page-image"),{
		onChange: showPreview,
		onSelect: showPreview,
		trueSize: truesize
	});
	jCrop.setSelect(peCoords);
}

$.fn.pageImageResize = function(options) {
	settings = $.extend({
		maxWidth: $(".page-image-wrapper").width()-15,
		realWidth: truesize[0],
		realHeight: truesize[1],
		pageImage: $(this),
		pageImageMask: $("#page-image-mask"),
		pixelMask: $("#pixel-mask"),
		elements: true
	},options);
	
	return this.each(function()
	{
		var currentWidth;
		currentWidth = settings.pageImage.attr("width");
		if(settings.realWidth > settings.maxWidth || (currentWidth < settings.maxWidth && currentWidth <= settings.realWidth))
		{			
			if(settings.realWidth < settings.maxWidth){
				settings.pageImage.attr("width",settings.realWidth);	
				settings.pageImage.attr("height",settings.realHeight);
				if(settings.pageImage.attr("width")){
					settings.pageImageMask.css({
						width: settings.pageImage.attr("width") + 'px',
						height: settings.pageImage.attr("height") + 'px',
					});
				}
				else{
					settings.pageImageMask.css({
						width: settings.realWidth + 'px',
						height: settings.realHeight + 'px',
					});
				}
			} else {
				settings.pageImage.attr("width",settings.maxWidth);	
				settings.pageImage.attr("height",settings.pageImage.attr("width")*settings.realHeight/settings.realWidth);
				if(settings.pageImage.attr("width")){
					settings.pageImageMask.css({
						width: settings.pageImage.attr("width") + 'px',
						height: settings.pageImage.attr("height") + 'px',
					});
				}
				else{
					settings.pageImageMask.css({
						width: settings.maxWidth + 'px',
						height: settings.maxWidth*settings.realHeight/settings.realWidth + 'px',
					});
				}
			}
			
			
			settings.pixelMask.css({
				width: settings.pageImage.attr("width") + 'px',
				height: settings.pageImage.attr("height") + 'px',
			});
		}
		else
		{
			settings.pageImageMask.css({
				width: settings.pageImage.attr("width") + 'px',
				height: settings.pageImage.attr("height") + 'px',
		});
		}
		if(settings.elements)
			settings.pageImageMask.drawPageElements();
		
	}
	);
}

$.fn.drawPageElements = function(options) {
	settings = $.extend({
		elements: page_elements,
		element_class: "page-element",
		typeList: $("#page-element-list"),
		realHeight: truesize[1],
		realWidth: truesize[0],
		currentHeight: $("#page-image").height(),
		currentWidth: $("#page-image").width()
		
	},options);
	var typeCount = 1;
	return this.each(function()
	{
		if(settings.elements == "No Elements Exist")
		{
			if(!$("#element-typenone").length)
				settings.typeList.append("<li id=\"element-typenone\"><a rel=\".pe1\" href=\"#\">No Elements Exist</a></li><ul>");
		}
		else 
		{
		var element,style, heightRatio, WidthRatio, elementList = "";
		for(var type in settings.elements )
		{		
   			for(var element_id in settings.elements[type])
   			{
   				heightRatio = settings.currentHeight/settings.realHeight;
				widthRatio = settings.currentWidth/settings.realWidth;
				
   				style = settings.elements[type][element_id];
   				
   				if($("#element-"+element_id).length)
   				{
   					$("#element-"+element_id).css({
   						top: (Math.round(style['top']*heightRatio)-1),
   						height: Math.round(style['height']*heightRatio),
   						left: (Math.round(style['left']*widthRatio)-1),
   						width: Math.round(style['width']*widthRatio),
   					});
   					if(style['mask'] != null)
   					{
   						$("#element-"+element_id).css('overflow','hidden');
   						$("#element-"+element_id+" img").css('width',settings.currentWidth)
   						$("#element-"+element_id+" img").css('height',settings.currentHeight);
   						$("#element-"+element_id+" img").css({
   						top: Math.round(style['top']*heightRatio*-1),
   						left: Math.round(style['left']*widthRatio*-1),
   					});
   					}
   				}
   				else
   				{
	   				element = "<div id=\"element-"+ element_id +"\" class=\""+ settings.element_class +" pe"+ typeCount + (style['mask'] != null?" pixel-mask":"") + "\" style=\"top:"+ Math.round(style['top']*heightRatio) +"px; left:"+ Math.round(style['left']*widthRatio) +"px; width:"+Math.round(style['width']*widthRatio) +"px; height:"+ Math.round(style['height']*heightRatio) +"px";
	   				
	   				if(style['mask'] != null)
	   				{
	   					element = element + "\"><img style=\"display:none;position:relative;top:-" + (Math.round(style['top']*heightRatio)+1) + "px;left:-"+  					(Math.round(style['left']*widthRatio)+1)+"px;width:"+ settings.currentWidth+"px;height:"+settings.currentHeight+"px;\" src=\""+Drupal.settings.basePath+"?q=browse/dataitem/pixel-mask/"+ selectedItem + "/" + style['mask'] + "\" alt=\"\" /></div>";
	   				}
	   				else
	   				{
	   					element = element + "\"></div>";
	   				}
	   				$(this).append(element);
	   				elementList = elementList + "<li><a rel=\"element-"+element_id+"\" href=\""+Drupal.settings.basePath+"?q=browse/dataitem/"+element_id+"\">" +style['name'] + "</a></li>";
	   			}
   			}
   			var typeName = type.replace(/[^a-zA-Z0-9]+/g,'');
   			if(!settings.typeList.find("#element-type-"+typeName).length)
				settings.typeList.append("<li id=\"element-type-"+typeName+"\"><a rel=\".pe"+typeCount+"\" href=\"#\"><span class=\"selected\"></span>"+type+"<span class=\"color pe"+typeCount+" hover\"></span></a></li><ul>"+ elementList +"</ul>");
   			elementList = "";
   			typeCount++;
		}
		}
	});

};
/*
	var config = {    
     over: pageElementZoomEffect,    
     timeout: 500,   
     out: pageElementZoomClose
	};
	
	$(".page-element").hoverIntent( config )
*/	
function pageElementControlsActivate()
{
		//show hide when clicking on a page element category
		$("#page-element-list li a:not(#page-element-list ul a)").click(function(){  //page element heading (show hide page element categories)
			if($($(this).attr("rel")).css("display") == "none") //if its hidden
			{
				$($(this).attr("rel")).css("display","block"); //show the bounding boxes
				$(this).find("span").addClass("selected"); //add check the heading
				$(this).parent().next().slideDown(); //show the list of page elements
			}else{ 
				$($(this).attr("rel")).css("display","none"); //hide the bounding boxes
				$(this).find("span").removeClass("selected"); //uncheck the heading
				$(this).parent().next().slideUp(); //hide the list of elements
			} 
			return false;
			});
		
		//highlight all page elements in the category when heading is hovered over
		$("#page-element-list li a").hover(function(){ 
			if($("div"+$(this).attr("rel")).hasClass('pixel-mask'))
				$("div"+$(this).attr("rel")+' img').css('display','block'); //add hover class to hovered element name
			else
				$("div"+$(this).attr("rel")).addClass("hover");		
		},function()
		{ 
			if($("div"+$(this).attr("rel")).hasClass('pixel-mask'))
				$("div"+$(this).attr("rel")+' img').css('display','none'); //add hover class to hovered element name
			else
				$("div"+$(this).attr("rel")).removeClass("hover"); //remove hover class on mouse out
		});
		
		//highlight individual page element when name is hovered over
		$("#page-element-list ul li a").hover(function(){
			if($("#"+$(this).attr("rel") + " img").size())
				$("#"+$(this).attr("rel") + " img").css('display','block');
			else
				$("#"+$(this).attr("rel")).addClass("hover"); //for bounding boxes
			
		},function()
		{
			if($("#"+$(this).attr("rel") + " img").size())
				$("#"+$(this).attr("rel") + " img").css('display','none');
			else
				$("#"+$(this).attr("rel")).removeClass("hover"); //for bounding boxes
			
		});
		
		$('.pixel-mask').hover(function(){
			$(this).css('background','transparent');
			$(this).css('opacity','1');
			$(this).children().css('display','block');
		},function(){
			$(this).children().css('display','none');
			$(this).css('opacity','inherit');
		});
		
		$(".page-element").click(function(){window.location = Drupal.settings.basePath+"?q=browse/dataitem/"+ this.id.substr(8);});
}
function displayPageElements()
{
	$(".page-element").fadeIn();
	$("#page-element-list").css("display","block");
	$("#page-element-list ul, #page-element-list span").css("display","block");
	$("#page-element-list span").addClass("selected");
	$(".page-image-wrapper").css({padding:'0 '+$("#page-element-list").width()+'px 0 0'});
	$("#page-image").pageImageResize();
	$("#page-element-list").animate({right: 0});
}

/**
 *  Adding Page Elements
 * 
 *  addPageElementClick - event handler for a click on the add page element button on the toolbar
 *  page-element-toolbar input change - event handler for users typing numbers into element coords boxes
 *  showPreview - mostly experimental, but also includes updaters for the numbers in coord boxes when
 *                element is resized using jCrop box
 *
 *
 */
function addPageElementClick(){

	$("#edit-dae-data-page-element-page-image").val(selectedItem);

	if(!addPageElements)
	{
		$("#page-element-toolbar").slideDown();
		//$("#page-element-preview").css("display","block");
		//$("#page-element-preview-wrapper").css("display","block");
		jCrop = $.Jcrop($("#page-image"),{
			onChange: showPreview,
			onSelect: showPreview,
			trueSize: truesize
		});
		addPageElements = true;
	}
	else
	{
		$("#page-element-toolbar").slideUp();
		//$("#page-element-preview").css({display: "none",width:"0px"});
		//$("#page-element-preview-wrapper").css("display","none");
		jCrop.destroy();
		addPageElements = false;
	}

}

$("#page-element-toolbar input").change(function(){
	var pos = [$("#edit-dae-data-page-element-x1").val(),$("#edit-dae-data-page-element-y1").val(),$("#edit-dae-data-page-element-x2").val(),$("#edit-dae-data-page-element-y2").val()];
	jCrop.animateTo(pos);
});


function showPreview(coords)
{
/* experimental zoom view of coords 
	var previewWindowWidth = 465; //480x270, but leave room for border
	var previewWindowHeight = 255;
	
	zoom = previewWindowHeight/coords.h;
	if(previewWindowWidth/coords.w < zoom)
		zoom = previewWindowWidth/coords.w;
	
	var height = Math.round(zoom*coords.h);
	var rx = height*(coords.w/coords.h) / coords.w;
	var ry = height / coords.h;
*/	
	$("#edit-dae-data-page-element-x1").val(coords.x);
	$("#edit-dae-data-page-element-x2").val(coords.x2);
	$("#edit-dae-data-page-element-y1").val(coords.y);
	$("#edit-dae-data-page-element-y2").val(coords.y2);
	peCoords = [$("#edit-dae-data-page-element-x1").val(),$("#edit-dae-data-page-element-y1").val(),$("#edit-dae-data-page-element-x2").val(),$("#edit-dae-data-page-element-y2").val()];
	
/*	
	pageImageWidth = truesize[0];
	pageImageHeight = truesize[1];
	
	$('#page-element-preview').css({
		width: Math.round(rx * pageImageWidth) + 'px',
		height: Math.round(ry * pageImageHeight) + 'px',
		marginLeft: '-' + Math.round(rx * coords.x) + 'px',
		marginTop: '-' + Math.round(ry * coords.y) + 'px'
	});
	$('#page-element-preview-mask').css({
		width: height*(coords.w/coords.h) + 'px',
		height: height + 'px',
		margin: (height/2)*-1 + 'px 0 0 0'
	});*/
}
/* also experimental 
function pageElementZoomEffect()
{
		var zoomSize = 30;
	
		$("#page-element-popout-wrapper").css({
			display: 'block',
			top: $(this).css("top"),
			left: $(this).css("left"),
			width: $(this).width() + 'px',
			height: $(this).height()+ 'px',
		});
		$("#page-element-popout img").css(
		{
			width: $("#page-image").attr("width") + 'px',
			height: $("#page-image").attr("height") +  'px',
			margin: "-" + $(this).css("top") + " 0 0 -" + $(this).css("left")
		});
		$("#page-element-popout").css(
		{
			width: $(this).width() + 'px',
			height: $(this).height()+ 'px',
		});
		var rx = ($(this).width()+zoomSize)/$(this).width();
		var ry = ($(this).height()+zoomSize)/$(this).height();
		
		$('#page-element-popout-wrapper').animate({
		 	top: $(this).css("top").substr(0,$(this).css("top").length-2)-zoomSize+10 + 'px',
			left: $(this).css("left").substr(0,$(this).css("left").length-2)-zoomSize+10 + 'px',
			width: $(this).width()+zoomSize + 'px',
			height: $(this).height()+zoomSize + 'px',
		  }, 300);
		  
		$('#page-element-popout img').animate({
		 	width: $("#page-image").attr("width")*rx + 'px',
			height: $("#page-image").attr("height")*ry +  'px',
			margin: "-" + $(this).css("top").substr(0,($(this).css("top").length)-2)*ry + "px 0 0 -" + $(this).css("left").substr(0,($(this).css("left").length)-2)*rx + "px"
		  }, 300); 
		$('#page-element-popout').animate({
		 	width: $(this).width()+zoomSize + 'px',
			height: $(this).height()+zoomSize + 'px',
		  }, 300);
		$("#page-image").css("opacity",".6");
		$(".page-image-wrapper").css("background", "#000000");

}
function pageElementZoomClose()
{
$("#page-image").css("opacity","1");
		$("#page-element-popout-wrapper").css("display","none");

}
*/
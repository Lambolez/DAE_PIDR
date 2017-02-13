ImageCrop = function(info) {
  
  var MarkupObject = function(info){
    this.id = info.id;
    this.type = info.type;
    this.coords = info.coords;
  }
  
  var lastTool,
  realWidth, realHeight, deleteButton, image, wrapper, showExistingButton, cancelButton,
  imgContainer, jCrop, markup, tags=[], tagDivs=[], saveButton, coordsFloat,
  markupFunctions = {
    rectangle : drawRectangle,
    circle : drawCircle
  };
  
  function imageResize() {
    if(jCrop) jCropDestroy();
    var currentWidth = image.attr('width') || realWidth,
    wrapperWidth = ($(document).height() <= window.innerHeight) ?
      wrapper.width()-15 : wrapper.width();
    
		if(realWidth > wrapperWidth ||
      (currentWidth < wrapperWidth &&
       currentWidth <= realWidth)){
			if(realWidth < wrapperWidth){
      	image.attr('width',realWidth);	
				image.attr('height',realHeight);
			} else {
				image.attr('width',wrapperWidth);	
				image.attr('height',wrapperWidth*realHeight/realWidth);
			}
      
      if(image.attr('width')){
        imgContainer.css({
					width: image.attr('width') + 'px',
					height: image.attr('height') + 'px'
				});
			} else {
				imgContainer.css({
					width: realWidth + 'px',
					height:realHeight + 'px'
			  });
			}
		}
  }
  
  function jCropDestroy() {
    jCrop.release();
    jCrop.destroy();
    coordsFloat.css({display: 'none'});
  }
  
  function setJcrop(coords, id) {
    var tagId = (id !== undefined) ? id : tags.length,
    hideExisting = imgContainer.hasClass('show-existing'),
    options = {
      setSelect: [0,0,0,0],
      onSelect: function(coords){updateJCropTags(coords, tagId, hideExisting)},
      onChange: function(coords){updateJCropTags(coords, tagId, hideExisting)},
      trueSize: info.truesize
    };
    
    if(coords) options.setSelect = [coords.x, coords.y, coords.x2, coords.y2];
    
    jCrop = $.Jcrop(image, options);
    
  }
  
  function buildCropUtils() {
    function getUtilClickFunction(i) {
      return function(){
        lastTool = info.tools[i];
        imgContainer.addClass('tooltip');
        markupFunctions[info.tools[i]].call();
      }
    }
    var toolContainer = $('<div id="image-crop-markup-tool-container"></div>');
    for(i in info.tools) {
      var button = $('<a class="image-crop-markup-tool" title="'+info.tools[i]+'"></a>');
      button.click(getUtilClickFunction(i));
     toolContainer.append(button);
    }
    wrapper.before(toolContainer);
  }
  
  function drawRectangle() {
    if(jCrop) jCropDestroy();
    setJcrop();
  }
  
  function drawCircle() {
    console.log('Circle is coming soon...');
  }
  
  function saveTag(coords, tagId) {
    var mObj = new MarkupObject({
      type: 'rectangle',
      id: 'a smaple id',
      coords: coords
    });
    tags[tagId] = mObj;
    if(!tagDivs[tagId]){
      tagDivs[tagId] = $('<div class="tag-div"></div>');
      imgContainer.append(tagDivs[tagId]);
    }
    tagDivs[tagId].css(resizeCoords(coords)).unbind('click').click(function(){setJcrop(resizeCoords(coords, true), tagId)});
    
    markup.val(JSON.stringify(tags));
    jCropDestroy();
  }
  
  function updateJCropTags(coords, tagId, hideExisting){
    
    if(hideExisting) imgContainer.removeClass('show-existing');
    
    var coordsHtml = '<table><tr>' +
    '<td class="bold">x1:</td><td>' + coords.x +
    '</td><td class="bold">y1:</td><td>' + coords.y + '</td></tr>' +
    '<td class="bold">x2:</td><td>' + coords.x2 +
    '</td><td class="bold">y2:</td><td>' + coords.y2+ '</td></tr></table>';

    if(!(coords.x == 0 && coords.w == 0)) {
      coordsFloat.html(coordsHtml);
      coordsFloat.append(saveButton.click(function(e){
        if(hideExisting) imgContainer.addClass('show-existing');
        e.preventDefault();
        saveTag(coords, tagId);
      })).append(cancelButton.click(function(e){
        if(hideExisting) imgContainer.addClass('show-existing');
        e.preventDefault();
        jCropDestroy();
      })).append(deleteButton.click(function(e){
        if(hideExisting) imgContainer.addClass('show-existing');
        e.preventDefault();
        deleteTag(tagId);
      }));
      coordsFloat.css({
        top: coords.y*(imgContainer.height()/realHeight) + 'px',
        left: ((coords.x)*(imgContainer.width()/realWidth) - coordsFloat.width() - 15) + 'px',
        display: 'block'
      });
    } else if(coords.w === 0 || coords.h === 0) {
      coordsFloat.css({display: 'none'});
    }
  }
  
  function deleteTag(tagId) {
    if(tags[tagId]) {
      delete tags[tagId];
      tagDivs[tagId].remove();
    }
    markup.val(JSON.stringify(tags));
    jCropDestroy();
  }
  
  function showExisting() {
    showExistingButton.toggleClass('off');
    imgContainer.toggleClass('show-existing');
  }
  
  function handleImageContainerClick(e) {
    if(lastTool && $(e.target).attr('id') == 'image-crop-image'){
      markupFunctions[lastTool]();
    }
  }
  
  function drawTagDivs() {
    for(var i in tagDivs) {
      if(tags[i]) {
        var coords = tags[i];
        tagDivs[i].css(resizeCoords(coords));
      }
    }
  }
  
  function resizeCoords(coords, opt_setForJcrop) {
    if(opt_setForJcrop){
      var rect = {
        x: coords.x*(imgContainer.width()/realWidth),
        y: coords.y*(imgContainer.height()/realHeight),
        x2: coords.x2*(imgContainer.width()/realWidth),
        y2: coords.y2*(imgContainer.height()/realHeight)
      };
      console.log(rect);
      return rect;
    }
    return {
      left: coords.x*(imgContainer.width()/realWidth),
      top: coords.y*(imgContainer.height()/realHeight),
      height: coords.h*(imgContainer.height()/realHeight),
      width: coords.w*(imgContainer.width()/realWidth)
    }
  }
  
  function init() {
    realWidth = info.truesize[0];
    realHeight = info.truesize[1];
    image =$('<img id="image-crop-image"/>');
    wrapper = $('#' + info.wrapper);
    imgContainer = $('<div id="image_container"></div>').append(image).addClass('show-existing');
    markup = $('#edit-tags');
    coordsFloat = $('<div id="coords-float"></div>');
    saveButton = $('<div id="save-tag-button">Save</div>');
    cancelButton = $('<div id="cancel-tag-button">Cancel</div>');
    deleteButton = $('<div id="delete-tag-button">Delete</div>');
    
    showExistingButton = $('<div id="show-existing-button" title="Toggle outlines"></div>').append('<div></div>');
    showExistingButton.click(showExisting);
    wrapper.before(showExistingButton);
    
    imgContainer.append(coordsFloat.css({display: 'none'}));
    
    image.attr('src' , info.image);
    wrapper.append(imgContainer).css({
      background : 'images/image_crop_bg.jpg'
    });
    
    $(window).resize(function(){
      imageResize();
      drawTagDivs();
      imgContainer.addClass('show-existing');
    });
    
    imageResize();
    imgContainer.mousedown(handleImageContainerClick);
    buildCropUtils();
    
    $(document).keypress(function(e){
      code = (e.keyCode? e.keyCode : e.whcih);
      if(code == 13) e.preventDefault();
    });
    
  };
  
  
  init();
};


newImageCrop = function(info){
  $(document).ready(function(){
    new ImageCrop(info);
  });
};
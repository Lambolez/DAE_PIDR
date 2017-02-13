/**
 * @file dae_data_browse.js
 * 
 * @author  Kevin Elko <kjelko@gmail.com>
 *
 * @description Provides the class for data browsing UI effects.
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
(function(){ //this closure prevents namespace leakage
  
var DAE = DAE || {};

/**
 * This is the constructor for the DAE.ui class.
 * It sets all of the class variables.
 * @constructor
 */
DAE.ui = function(){
  if(!this.isPage('home')){
    if(this.isPage('browse')) {
      this.item = selectedItem;
      this.toolbar = $('.dataset-toolbar-wrapper');
      this.listContainer = $('.item-container');//$('#dataset-item-container');
      this.breadCrumb = $('#toolbar-breadcrumb');
      this.topButton = this.buildTopButton();
      this.setUpBreadCrumb();
      this.setUpScroll();
    }
    this.buildSubList();
  }
};

/**
 * Determines if the page is the root dataset page or not.
 * @param page type of page to check
 * @return returns true if the current page is of the given type
 */
DAE.ui.prototype.isPage = function(page) {
  if(page == 'browse') {
    return (document.location+'').match('.*?q=browse/dataitem/.*');
  } else if(page == 'home') {
    return (document.location+'').match('.*?q=browse$');
  } else {
    return false;
  }
};

/**
 * Makes a variety of important urls available
 * @param toGet the type of url to get
 * @param endUrl appended to end of the url
 * @return returns the desired url
 */
DAE.ui.getPath = function(toGet, endUrl) {
  if(toGet == 'uri') {
    return Drupal.settings.basePath + '?q=browse/dataitem/thumb/uri/' + endUrl;
  } else if (toGet == 'list') {
    return Drupal.settings.basePath + '?q=browse/dataitem/list/' + endUrl;
  } else if(toGet == 'browse-full') {
    return Drupal.settings.basePath + '?q=browse/dataitem/' + endUrl;
  } else if(toGet == 'item-info') {
    return Drupal.settings.basePath + '?q=browse/dataitem/dataitem/' + endUrl;
  } else if(toGet == 'browse') {
    return Drupal.settings.basePath + '?q=browse';
  } else if(toGet == 'tree') {
    return Drupal.settings.basePath + '?q=browse/tree/' + endUrl;
  }
};

/**
 * This function initializes the sublist UI functions.
 */
DAE.ui.prototype.buildSubList = function() {
  var ui = this
  $('.single-item').mouseover(function(){
    var itemInfo = $(this).children('.item-info');
    var itemId = this.id.substr(9);
    var el = $(this).offset().top + 325;
    var wi = window.pageYOffset + window.innerHeight;
    ui.itemHover(itemInfo, itemId, el, wi);
  });
  $('.load-more').click(function(){
    var datasetId = this.id.substr(17);
    var page = parseInt(this.title) + 1;
    $(this).children('.hidden').removeClass('hidden');
    $(this).children('.load-more-span').addClass('hidden');
    var t = this;
    $.get(DAE.ui.getPath('list', datasetId+'/'+page), function(data){
      $(t).remove();
      ui.loadMore(data);
    });
  });
};

/**
 * Sets scroll event listener and locks toolbar to the top of the
 * page if far enough down.
 */
DAE.ui.prototype.setUpScroll = function() {
  var ui = this;
  var toolbarTop = this.toolbar.offset().top;
  $(document).scroll(function(){
    if($(window).scrollTop() > toolbarTop) {
      ui.setScrollCss();
      ui.topButton.css({display:'block'});
    } else {
      ui.toolbar.removeAttr('style');
      ui.topButton.css({display:'none'});
    }
  });
  if($(window).scrollTop() > toolbarTop) {
      ui.setScrollCss();
      ui.topButton.css({display:'block'});
  }
  $(window).resize(function(){
    if($(window).scrollTop() > toolbarTop){
      ui.setScrollCss();
    }
  });
};

/**
 * Sets the toolbar CSS
 */
DAE.ui.prototype.setScrollCss = function(){
  var toolbarLeft = this.listContainer.offset().left;
  var toolbarWidth = this.listContainer.width();
  this.toolbar.css({
    'position' : 'fixed',
    'top' : '-1px',
    'width' : toolbarWidth+'px',
    'left' : toolbarLeft+'px',
    'z-index' : '999'
  });
};

/**
 * Calls thge tree function to build the breadcrumb
 */
DAE.ui.prototype.setUpBreadCrumb = function(){
  var ui = this;
  var currentSet = this.breadCrumb.html();
  var homeA = $('<a href="'+ DAE.ui.getPath('browse') +'">Home</a>');
  this.breadCrumb.html(homeA);
  var treePath = DAE.ui.getPath('tree', this.item);
  var output = '';
  $.getJSON(treePath, function(data) {
    if(data){
      var containedBy = data.cb;
    }
    if(containedBy) {
  	  for(item in containedBy) {
          var name = containedBy[item].name;
          if(name.length > 15){
            name = containedBy[item].name.substr(0,12) + '...';
          }
          output = ' &rsaquo; <a href="'+
            DAE.ui.getPath('browse-full', containedBy[item].id)+'">'+
            name+'</a>' + output;
      }
    }
    ui.breadCrumb.append(output);
    ui.breadCrumb.append(' &rsaquo; ' + currentSet);
  });
};

/**
 * Handles the onmouseover events for elements
 */
DAE.ui.prototype.itemHover = function(e, id, el, wi){
  if (e.children().length == 1) {
    $.get(DAE.ui.getPath('item-info', id), function(d){
      e.html(d);
    });
  }
  if(el > wi) {
    $('html,body').animate(({
      scrollTop : window.pageYOffset+el-wi+10
    }), 100);
  }
};

/**
 * Loads more elements to the page
 */
DAE.ui.prototype.loadMore = function(d) {
  this.listContainer.append(d);
  this.buildSubList();
};


/**
 * Builds the 'Top' button and appends it to the toolbar
 */
DAE.ui.prototype.buildTopButton = function() {
  var topButton = $('<span>&uarr; Top</span>');
  topButton.attr('id','toolbar-up');
  topButton.css({display:'none'});
  topButton.click(function(){
    $('body').animate({scrollTop:0}, 'fast');
  });
  this.toolbar.append(topButton);
  return topButton;
}

/**
 * Calls the constructor of the DAE.ui when page is ready
 */
$(document).ready(function(){
  var ui = new DAE.ui();
});

})()
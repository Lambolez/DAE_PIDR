var DAE = DAE || {};

DAE.AnnotationUI = function(info){
  this.title = info.annotation_name;
  this.output = $(info.output).show();
  (this.console = $(info.console)).html('');
  this.toolsAvail = info.toolsAvail;
  this.image = info.image;
  this.widgets = [];
  this.setUpPopup();
  this.setUpToolbar(info.widgetTypes);
  this.setUpPreview();
  this.query = info.query;
  if(info.exists){
    this.buildFromExisting(info.annotation_id);
  } else {
    this.documentWidget = new DAE.AnnotationUI.DocumentWidget(this.image);
    this.widgets.push(this.documentWidget);
    this.updateOutput();
  }
};

DAE.AnnotationUI.prototype.buildFromExisting = function(id) {
  var that = this;
  this.preview.html('<br>Loading ' + this.title + '...<br><br>');
  function buildWidget(xmlspec) {
    var type = xmlspec.attr('type');
    if(type != 'markup'){
      var widget = new DAE.AnnotationUI.Widget(type);
      var name = xmlspec.attr('name');
      var content = [];
      if(widget.hasChildren()){
        xmlspec.children('option').each(function(){
          content.push($(this).text());
        });
      } else {
        content.push(xmlspec.text());
      }
      widget.update({
        'name' : name,
        'content' : content
      });
      that.widgets.push(widget);
    } else {
      that.documentWidget = new DAE.AnnotationUI.DocumentWidget(that.image);
      var tools = [];
      xmlspec.children('tool').each(function(){
        tools.push($(this).attr('type'));
      });
      that.documentWidget.update(tools);
      that.widgets.push(that.documentWidget);
    }
  }
  $.get('?q=annotations/specification/' + id, function(spec){
    $(spec).find('widget').each(function(){
      buildWidget($(this));
    });
    that.updateOutput();
  }, 'xml');
};

DAE.AnnotationUI.prototype.setUpPopup = function(){
  var that = this;
  this.popup = $(extras.popup.outer);
  this.popupContent = $(extras.popup.content);
  this.popup.append(this.popupContent);
  $('body').append(this.popup);
};

DAE.AnnotationUI.prototype.setUpToolbar = function(types){
  var that = this;
  function addEvent(button, type){
    button.click(function(){that.newWidget(type)});
  }
  
  this.toolbar = $(extras.toolbar);
  for(i in types){
    var button = $(extras.toolbarButton).append(types[i]);
    addEvent(button, types[i]);
    this.toolbar.append(button);
  }
  var tools = $(extras.toolbarButton).addClass('edit-tools').append('Edit Markup Tools');
  tools.click(function(){that.toolsContentPrompt()})
  this.toolbar.append(tools);
  this.console.append(this.toolbar);
};

DAE.AnnotationUI.prototype.setUpPreview = function(){
  var that = this;
  this.preview = $(extras.previewHTML);
  this.preview.sortable({
    'containment' : that.preview,
    'axis' : 'y',
    'stop' : function(){that.refreshAfterSort(this)}
  });
  this.console.append(this.preview);
};

DAE.AnnotationUI.prototype.submitContent = function(widget){
  var name = $('.' + extras.classes.widgetName).val().replace(/\s+/g, '-');
  var values = {
   'name'     : (name ? name : this.defaultWidgetName()),
   'content'  : []
  }
  if(this.getWidgetById(values.name) != false && 
    this.getWidgetById(values.name) != widget){
    alert('Sorry, that name is already used.');
    return;
  }
  $('.' + extras.classes.widgetValue).each(function(){
    if($(this).val() != '') {
      values.content.push($(this).val());
    }
  });
  widget.update(values);
  if(this.widgets.indexOf(widget) == -1) {
    this.widgets.push(widget);
  }
  this.updateOutput();
  this.popup.addClass(extras.classes.hidden);
};

DAE.AnnotationUI.prototype.defaultWidgetName = function() {
  return 'new-widget-' + this.widgets.length;
};

DAE.AnnotationUI.prototype.newWidget = function(type){
  var widgetTag = type.replace(' ', '').toLowerCase();
  var widget = new DAE.AnnotationUI.Widget(widgetTag);
  this.widgetContentPrompt(widget, type);
};

DAE.AnnotationUI.prototype.widgetContentPrompt = function(widget){
  var that = this;
  buildSubmitionEvent = function(widget) {
    var popupClose = $(extras.doneButton);
    popupClose.click(function(){that.submitContent(widget)});
    return popupClose;
  };
  
  buildDeleteEvent = function(widget) {
    var deleteWidget = $('<button class="delete-from-popup">Delete this widget</button>');
    deleteWidget.click(function(){
      that.deleteWidget(widget);
      that.popup.addClass(extras.classes.hidden);
    });
    return deleteWidget;
  }
  
  this.popupContent.html(widget.getContentForm());
  this.popupContent.append(buildSubmitionEvent(widget));
  this.popupContent.append(buildDeleteEvent(widget));
  this.popup.removeClass(extras.classes.hidden);
  $('.widget-name-input').focus();
};

DAE.AnnotationUI.prototype.toolsContentPrompt = function(){
  var that = this;
  buildSubmitionEvent = function() {
    var popupClose = $(extras.doneButton);
    popupClose.click(function(){that.submitToolsContent()});
    return popupClose;
  };
  this.popupContent.html(this.documentWidget.getContentForm(that.toolsAvail));
  this.popupContent.append(buildSubmitionEvent());
  this.popup.removeClass(extras.classes.hidden);
};

DAE.AnnotationUI.prototype.submitToolsContent = function(){
 var tools = [];
  $('.tools-option:checked').each(function(){
    tools.push($(this).val())
  });
  this.documentWidget.update(tools);
  this.updateOutput();
  this.popup.addClass(extras.classes.hidden);
};

DAE.AnnotationUI.prototype.deleteWidget = function(widget){
  delete this.widgets[this.widgets.indexOf(widget)];
  this.updateOutput();
};

DAE.AnnotationUI.prototype.buildHTMLWidgetControls = function(widget) {
  var that = this;
  var output = widget.getHTML();
  var closeButton = $(extras.widgetControl).append('X').click(function(){
    that.deleteWidget(widget);
  });
  var editButton = $(extras.widgetControl).append('E').addClass(extras.classes.editWidget);
  editButton.click(
    function(){
      that.widgetContentPrompt(widget);
    }
  );
  var name = $('<div class="widget-name"></div>').append(widget.getName());
  return output.append(name).append(closeButton).append(editButton);
};

DAE.AnnotationUI.prototype.updateOutput = function(){
  var that = this;
  var markupOutput = '<' + extras.markupParent + '>';
  this.preview.html('');
  for(i in this.widgets){					
    var widget = this.widgets[i];
    markupOutput += '\n\t' + widget.getMarkup();
    if(widget != this.documentWidget) {
      this.preview.append(this.buildHTMLWidgetControls(widget));
    } else {
      this.preview.append(this.documentWidget.getHTML());
    }
  }
  markupOutput += '\n</' + extras.markupParent + '>';
  this.output.val(markupOutput); 
  if(this.preview.html() == ''){
    this.preview.html(extras.immediateInst);
    this.preview.sortable('disable');
  } else {
    this.preview.sortable('enable');
  }
};

DAE.AnnotationUI.prototype.refreshAfterSort = function(){
  var i = 0, that = this, newWidgetArray = [];
  this.preview.children('.widget').each(function(){
    newWidgetArray.push(that.getWidgetById(this.id));
  });
  this.widgets = newWidgetArray;
  this.updateOutput();
};

DAE.AnnotationUI.prototype.getWidgetById = function(id) {
  for(i in this.widgets){
    if(this.widgets[i].getName() == id) {
      return this.widgets[i];
    };
  }
  return false;
};

/*
DAE.AnnotationUI.prototype.getTitle = function(){
  return this.name;
};

DAE.AnnotationUI.prototype.getQuery = function(){
  return this.query;
};
*/

//============= Begin Widget Class Definition =============

DAE.AnnotationUI.Widget = function(type){
  var that = this, content = [], name = '';
  
  this.update = function(update){
    name = update.name;
    content = update.content;
  };
  
  this.getMarkup = function(){
    var output;
    if(this.hasChildren()){
      output = '<widget type="' + type + '" name="' + name + '">';
      for(i in content){
        output += '\n\t\t<' + extras.mrkupWidgtChld + '>' + content[i] 
          + '</' + extras.mrkupWidgtChld + '>';
      }
      output += '\n\t</widget>';
    } else {
      output = '<widget type="' + type + '" name="' + name + '">' + (content[0] ? content[0] : '') 
        + '</widget>';
    }
    return output;
  };
  
  this.getHTML = function(){
    var output = $('<div class="widget ' + type + '" id="' + name + '"></div>');
    if(this.hasChildren(type)){
      if(content.length != 0){
        for(i in content){
          var o = $(extras.wdgtOptHTML);
          output.append(o.append('<input type="' + type + '"/>' + content[i]));
        }
      } else {
        output.append('Click here to add options');
      }
    } else {
      output.append((content[0] ? content[0] : '<em>No text added</em>'));
    }
    return $(output);
  };
  
  this.getContentForm = function(){
    var form = $('<div class="popup-form"></div>').append('<div class="' + extras.classes.wdgtNameCont + 
      '">Name: <input type="textbox" class="' + extras.classes.widgetName + '" value="' +
      name + '"/><br / ><span class="widget-name-instr">Names must be unique and not contain spaces (spaces will be replaced by \'-\').</span>');
    if(this.hasChildren()){
      var sortCont = $('<div></div>');
      sortCont.sortable({
        'containment' : sortCont,
        'axis' : 'y'
      });
      if(content && content.length > 0) {
        for(i in content){
          var o = $(extras.blankOption);
          o.find('input').val(content[i]);
          sortCont.append(o);
        }
      } else {
        sortCont.append(extras.blankOption + extras.blankOption + extras.blankOption);
      }
      form.append(sortCont);
      form.append($(extras.moreFields).click(function(){
        sortCont.append(extras.blankOption + extras.blankOption + extras.blankOption);
      }));
    } else {
      form.append((type == 'textarea' ? 'Label' : 'Text') 
        + ': <textarea class="widget-value">' + 
        (content ? content : '') + '</textarea>');
    }
    return form;
  };
  
  this.getName =function(){
    return name;
  };
  
  this.hasChildren = function() {
    if(type == 'statictext' || type == 'textarea') {
      return false
    } else {
      return true;
    }
  };
  
};

DAE.AnnotationUI.DocumentWidget = function(dataitem) {
  type = 'markup';
  tools = ['rectangle'];
  
  this.update = function(newTools) {
    tools = newTools;
  };
  
  this.getHTML = function(){
    var widget = $(extras.docWidget).attr('id', dataitem);
    widget.append('<img src="' + extras.imgUrl + dataitem + '/medium" >');
    widget.append(this.getToolsHTML());
    return widget;
  };
  
  this.getToolsHTML = function(){
    var o = '<div id="annotation-tools-container"><div><h3>' + extras.toolsTitle + '</h3>';
    for(i in tools){
      o += '<span class="annotation-tool">' + tools[i] + '</span></br>';
    }
    o += '</div></div>';
    return o;
  };
  
  this.getMarkup = function() {
    var output = '<widget type="' + type + '">';
    for(i in tools){
      output += '\n\t\t<tool type="' + tools[i] + '"></tool>';
    }
    output += '\n\t</widget>';
    return output;
  };
  
  this.getName = function(){
    return dataitem;
  };
  
  this.getContentForm = function(toolsAvail){
    var output = '<div id="tools-options-container">';
    for(i in toolsAvail){
      output += '<input class="tools-option" type="checkbox" value="' + toolsAvail[i] + '" ';
      output += (tools.indexOf(toolsAvail[i]) == -1) ? '' : 'checked';
      output += ' />'+toolsAvail[i]+'<br />';
    }
    return output + '</div>';
  };
}

//================ End DAE.AnnotationUI Class =================

extras = {
  'markupParent'  : 'annotation_constraint',
  'mrkupWidgtChld': 'option',
  'toolbar'       : '<div id="annotation-toolbar"></div>',
  'toolbarButton' : '<div class="new-widget-button"></div>',
  'widgetControl' : '<div class="widget-control"></div>',
  'blankOption'   : '<div class="option">Option: <input type="textfield" class="widget-value"/></div>',
  'immediateInst' : '<span id="annotation-immediate-instr">Use the buttons above '+
                    'to add widgets to your annotation interface</span>',
  'previewHTML'   : '<div id="annotation-gui-preview"></div>',
  'doneButton'    : '<button id="close-annotation-popup">Done</button>',
  'moreFields'    : '<button>More fields</button>',
  'wdgtOptHTML'   : '<span class="widget-html-option"></span>',
  'docWidget'     : '<div class="widget document"></div>',
  'imgUrl'        : 'http://dae.cse.lehigh.edu/DAE/?q=browse/dataitem/thumb/',
  'toolsTitle'    : 'Markup Tools: ',
  'popup'         : {
                    'outer'       : '<div id="annotation-popup-outer" class="hidden"></div>',
                    'title'       : '<div id="annotation-popup-title"></div>',
                    'content'     : '<div id="annotation-popup-content"></div>'
                    },
  'classes'       : {
                    'editWidget'  : 'edit-widget',
                    'hidden'      : 'hidden',
                    'widgetName'  : 'widget-name-input',
                    'widgetValue' : 'widget-value',
                    'wdgtNameCont': 'widget-name-container'
                    }
}

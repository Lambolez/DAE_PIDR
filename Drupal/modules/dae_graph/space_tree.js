
$(document).ready(function() {

	$(".infovis_anchor_bis").bind('click', function() {
  	window.location.href = this.href;
  	return false;
	});

	
    	$(".infovis_anchor_bis").trigger('click');

	
});






var labelType, useGradients, nativeTextSupport, animate;


//Array to stock all necessary json
var json = new Array();

(function() {
  var ua = navigator.userAgent,
      iStuff = ua.match(/iPhone/i) || ua.match(/iPad/i),
      typeOfCanvas = typeof HTMLCanvasElement,
      nativeCanvasSupport = (typeOfCanvas == 'object' || typeOfCanvas == 'function'),
      textSupport = nativeCanvasSupport 
        && (typeof document.createElement('canvas').getContext('2d').fillText == 'function');
  //I'm setting this based on the fact that ExCanvas provides text support for IE
  //and that as of today iPhone/iPad current text support is lame
  labelType = (!nativeCanvasSupport || (textSupport && !iStuff))? 'Native' : 'HTML';
  nativeTextSupport = labelType == 'Nat$(document).ready(StickyRotate.init);ive';
  useGradients = nativeCanvasSupport;
  animate = !(iStuff || !nativeCanvasSupport);
})();




function init(location,json_input){


    //init data

	 json["location"]= JSON.parse(json_input);

  
    
    //preprocess subtrees orientation
    var arr = json["location"].children, len = arr.length;
    for(var i=0; i < len; i++) {
    	//split half left orientation
      if(arr[i].id.indexOf("=>") != -1) {
    		arr[i].data.$orn = 'left';
    		$jit.json.each(arr[i], function(n) {
    			n.data.$orn = 'left';
    		});
    	} else {
    	//half right
    		arr[i].data.$orn = 'right';
    		$jit.json.each(arr[i], function(n) {
    			n.data.$orn = 'right';
    		});
    	}
    }
    //end

    //init Spacetree
    //Create a new ST instance
    var st = new $jit.ST({
        //id of viz container element
        injectInto: location,
        //multitree
    	  multitree: true,
        //set duration for the animation
        duration: 200,
        //set animation transition type
        transition: $jit.Trans.Quart.easeInOut,
        //set distance between node and its children
        levelDistance: 40,
        //sibling and subtrees offsets
        siblingOffset: 3,
        subtreeOffset: 3,
        //set node and edge styles
        //set overridable=true for styling individual
        //nodes or edges
        Node: {
            height: 45,
            width: 110,
            type: 'ellipse',
            color: '#FFFFFF',
            overridable: true,
            //set canvas specific styles
            //like shadows
            CanvasStyles: {
              shadowColor: '#FFFFFF',
              shadowBlur: 10
            }
        },
        Edge: {
            type: 'line',
            overridable: true
        },
        
       

 	// Add node events  
    Events: {  
    enable: true,  
    type: 'auto', 
	//Change cursor style when hovering a node  
    onMouseEnter: function() {  
      fd.canvas.getElement().style.cursor = 'move';  
    	},  
    onMouseLeave: function() {  
      fd.canvas.getElement().style.cursor = '';  
    	},  

	},
        
        //This method is called on DOM label creation.
        //Use this method to add event handlers and styles to
        //your node.
        onCreateLabel: function(label, node){
            label.id = node.id;  

            label.innerHTML = node.name.substring(0,20);
            label.onclick = function(){
    if(true){
            	 if ( $jit.id('inner-details-'+location) != undefined){

		var html= "<strong>	 Algorithm:</strong> "+node.name+"<strong> Id Run: </strong>"+node.data["run"]
+"<strong> Input:</strong> "+
"<a href=./?q=browse/dataitem/"+node.data["input"]+">"+node.data["input"]+"</a>"
+"<strong> Output:</strong> "+
"<a href=./?q=browse/dataitem/"+node.data["output"]+">"+node.data["output"]+"</a>";
       
		$jit.id('inner-details-'+location).innerHTML = html;  
}
                st.onClick(node.id);
            	} else {
                st.setRoot(node.id, 'animate');
            	}
            };
            //set label styles
            var style = label.style;
            style.width = 110 + 'px';
            style.height = 45 + 'px';            
            style.cursor = 'pointer';
            style.color = '#333';
            style.fontSize = '0.8em';
            style.textAlign= 'center';
            style.paddingTop = '10px';
        },
        
        //This method is called right before plotting
        //a node. It's useful for changing an individual node
        //style properties before plotting it.
        //The data properties prefixed with a dollar
        //sign will override the global node style properties.
        onBeforePlotNode: function(node){
            //add some color to the nodes in the path between the
            //root node and the selected node.
            if (node.selected) {
                node.data.$color = "#FFFFFF";
            }
            else {
                delete node.data.$color;
                //if the node belongs to the last plotted level
              
                    //count children number
                    var count = 0;
                    node.eachSubnode(function(n) { count++; });
                    //assign a node color based on
                    //how many children it has
                    node.data.$color = ['#A9E2F3', '#81DAF5', '#58D3F7', '#2ECCFA', '#00BFFF', '01A9DB'][count];                    
                
            }
        },

        //This method is called right before plotting
        //an edge. It's useful for changing an individual edge
        //style properties before plotting it.
        //Edge data proprties prefixed with a dollar sign will
        //override the Edge global style properties.
        onBeforePlotLine: function(adj){
            if (adj.nodeFrom.selected && adj.nodeTo.selected) {
                adj.data.$color = "#eed";
                adj.data.$lineWidth = 3;
            }
            else {
                delete adj.data.$color;
                delete adj.data.$lineWidth;
            }
        }
    });


    //load json data
    st.loadJSON(json["location"]);
    //compute node positions and layout
    st.compute('end');
    st.select(st.root);
    //end
}


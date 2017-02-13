function my_ajax(dn,val){
	var obj=document.getElementById(dn);
	if(obj!=null){
		obj.innerHTML='<center>loading...</center>';
 	
 		$.ajax({
   				type: "GET",
				url: "./?q=browse/query_data_form/"+dn+"/"+val,
   				success: function(msg){
					eval(msg);
   				}
 			});
 	}
 		
}

function change_count(){
	var val=document.getElementById("v_count").value;
	jPrompt("How many at most the query should return?<br/>(You can input 0 to indicate no limit)",val,"Count Limit",
		function(r){ 
			if(r.replace(/\d+/,"").length==0){
				if(r.charAt(0)=='0'){
					r=r.replace(/0+/,"");
					if(r.length==0){
						r="0";
					}
				}
				document.getElementById("v_count").value=r;
				count_text();
			}else{
				alert("Invalid Input!");
			}
		});
}
function count_text(){
	var r=document.getElementById("v_count").value;
	var str="At Most "+r+" Random";
	if(r==0){
		str="ALL";
	}
	document.getElementById("a_count").innerHTML=str;
}

function str2Arr(str){
	var arr=[];
	if(str!="[]"){
		var nstr=str.substring(1,str.length-1);
		if(nstr.trim().length>0){
			arr=nstr.split(",");	 
		}
	}
	return arr;
}
function arr2Str(arr){
	var sz="[";
	for(i=0;i<=arr.length-1;i++){
		sz+=arr[i];
		if(i<arr.length-1){
			sz+=",";
		}
	}
	sz+="]";
	return sz;
}

function adding_vect(vid,aid,did){
	var sz=document.getElementById(vid).value;
	var vect=str2Arr(sz);
	if(vect[vect.length-1]!="-1"){
		vect.push("-1");
		var val=arr2Str(vect);
		document.getElementById(vid).value=val;
		vect_text(vid,aid,did);
	}
}

function added_vect(vid,aid,did,tmpid){
	var sz=document.getElementById(vid).value;
	var vect=str2Arr(sz);
	if(vect[vect.length-1]=="-1"){
		var nv=document.getElementById(tmpid).value;
		if(nv.trim().length==0){
			alert("You have nothing to add due to the constraints; or you will get empty result. Please remove this. ");
		}else{
			vect[vect.length-1]=document.getElementById(tmpid).value;
			var val=arr2Str(vect);
			document.getElementById(vid).value=val;
			notify_all_but(vid);
			vect_text(vid,aid,did);
		}
	}
}

function notify_all_but(vid){
	if(need_notify('v_pitype',vid)){
		vect_text('v_pitype','a_pitype','div_pitype');
	}
	if(need_notify('v_dataset',vid)){
		vect_text('v_dataset','a_dataset','div_dataset');
	}
	if(need_notify('v_petype',vid)){
		vect_text('v_petype','a_petype','div_petype');
	}
	if(need_notify('v_pedtprop',vid)){
		vect_text('v_pedtprop','a_pedtprop','div_pedtprop');
	}
}

function need_notify(vid,exvid){
	if(vid==exvid){
		return false;
	}
	var sz=document.getElementById(vid).value;
	var vect=str2Arr(sz);
	return (vect[vect.length-1]=="-1");
}

function remove_vect(vid,aid,did,ri){
	var sz=document.getElementById(vid).value;
	var vect=str2Arr(sz);
	var nv=[];
	for(i=0;i<=vect.length-1;i++){
		if(i!=ri){
			nv.push(vect[i]);
		}
	}
	var val=arr2Str(nv);
	document.getElementById(vid).value=val;
	vect_text(vid,aid,did);
}

function vect_text(vid,aid,did){
	var sz=document.getElementById(vid).value;
	var vect=str2Arr(sz);
	var cnt=vect.length;
	var str="One of the Following "+cnt+": ";
	if(cnt==0){
		str="ANY";
	}
	document.getElementById(aid).innerHTML=str;
	if(sz.length==2){
		document.getElementById(did).innerHTML='';
	}else{
		var asp="|";
		sz=document.getElementById('v_pitype').value+asp
			+document.getElementById('v_dataset').value+asp
			+document.getElementById('v_petype').value+asp
			+document.getElementById('v_pedtprop').value+asp
			+document.getElementById('l_pitype').value+asp
			+document.getElementById('l_dataset').value+asp
			+document.getElementById('l_petype').value+asp
			+document.getElementById('l_pedtprop');
		my_ajax(did,sz);
	}
}

function adding_val(vid,aid,did){
	
	jPrompt("Please input the value","","Adding a New Value",
		function(r){
			var rr=r.trim(); 
			if(rr.length>0){
				added_val(rr,vid,aid,did);
			}
		});
}
function added_val(r,vid,aid,did){
	var vsp=",";
	var ostr=document.getElementById(vid).value;
	var vals=str2VArr(ostr);
	var norep=true;
	var rr=my_encoder(r);
	for(i=0;i<=vals.length-1;i++){
		if(vals[i]==rr){
			norep=false;
			break;
		}
	}
	if(norep){
		var nval=ostr;
		if(nval.length>0){
			nval+=vsp;
		}
		nval+="'"+rr+"'";
		document.getElementById(vid).value=nval;
		val_text(vid,aid,did);
	}else{
		alert("This value has been input already!");
	}
}

function my_encoder(r){
	var rr=r;
	rr =encodeURIComponent(rr);
	return rr;
}

function my_decoder(r){
	var rr=r;
	rr =decodeURIComponent(rr);
	return rr;
}

function str2VArr(str){
	var vals=[];
	if(str.length>0){
		var vsp=",";
		var parts=str.substring(1,str.length-1).split("','");
		for(i=0;i<=parts.length-1;i++){
			vals.push(parts[i]);
		}
	}
	return vals;
}
function val_text(vid,aid,did){
	var vals=str2VArr(document.getElementById(vid).value);
	var astr="ANY";
	if(vals.length>0){
		astr="One of the following "+vals.length+": ";
	}
	document.getElementById(aid).innerHTML=astr;
	var dstr="<ul>";
	for(i=0;i<=vals.length-1;i++){
		dstr+="<li>"+my_decoder(vals[i])+" <a class=\"query_remove\" title=\"click to remove\" onclick=\"remove_val(\'"
			+vid+"\',\'"+aid+"\',\'"+did+"\',"+i+")\">X</a></li>"
	}
	dstr+="</ul>";
	document.getElementById(did).innerHTML=dstr;
}
function remove_val(vid,aid,did,ri){
	var vsp=",";
	var sz=document.getElementById(vid).value;
	var vect=str2VArr(sz);
	var nstr="";
	for(i=0;i<=vect.length-1;i++){
		if(i!=ri){
			nstr+="'"+vect[i]+"'";
			if(i<vect.length-2||(i==vect.length-2&&ri<vect.length-1)){
				nstr+=vsp;
			}
		}
	}
	document.getElementById(vid).value=nstr;
	val_text(vid,aid,did);
}

function selChange(tmpid){
	var hintid="hint_"+tmpid;
	var obj=document.getElementById(tmpid);
	document.getElementById(hintid).innerHTML=obj.options[obj.selectedIndex].title;
}

function query_now(){
	if(check_form()){
		var url_1="./query/page_image/";
		var url_2="./query_preview/page_image/";
		var sub_url="";
		sub_url+="?limit="+encodeURI(document.getElementById("v_count").value);
		sub_url+="&pitype="+encodeURI(document.getElementById("v_pitype").value);
		sub_url+="&dataset="+encodeURI(document.getElementById("v_dataset").value);
		sub_url+="&petype="+encodeURI(document.getElementById("v_petype").value);
		sub_url+="&pedtprop="+encodeURI(document.getElementById("v_pedtprop").value);
		sub_url+="&peval="+encodeURI(document.getElementById("v_peval").value);
		sub_url+="&lpitype="+encodeURI(document.getElementById("l_pitype").value);
		sub_url+="&ldataset="+encodeURI(document.getElementById("l_dataset").value);
		sub_url+="&lpetype="+encodeURI(document.getElementById("l_petype").value);
		sub_url+="&lpedtprop="+encodeURI(document.getElementById("l_pedtprop").value);
		
		url_1+=sub_url;
		url_2+=sub_url;
		
		//jAlert('SQL preview:<div id="ta"/><script>loadTA(\''+url_2+'\');</script>');
		jConfirm('SQL preview:<br/><textarea id="ta" readonly cols="50" rows="15"></textarea><script>loadTA("'+url_2+'");</script>', 
			'Ready to Run?', function(r) {
				if(r){
    				window.location=url_1;;
    			}
			});
	}
}

function loadTA(url_2){
	var obj=document.getElementById("ta");
	obj.value="generating your SQL...";
	$.ajax({
   				type: "GET",
				url: url_2,
   				success: function(msg){
					obj.value=msg;
   				}
 			});
}

function check_form(){
	if(check_vid("v_pitype","the types of page images")){
		if(check_vid("v_dataset","the datasets of page images")){
			if(check_vid("v_petype","the types of page elements")){
				if(check_vid("v_pedtprop","the property types of page elements")){
					return true;
				}
			}
		}
	}
	return false;
}

function check_vid(vid,vname){
	var sz=document.getElementById(vid).value;
	var vect=str2Arr(sz);
	if(vect[vect.length-1]=="-1"){
		alert("You need to add or delete the unfinished item for "+vname);
		return false;
	}else{
		return true;
	}
}

function logi_text(lid,aid,tid){
	var flag=document.getElementById(lid).value;
	var arr=document.getElementById(tid).value.split("|");
	if(flag>=0&&flag<=arr.length-1){
		document.getElementById(aid).innerHTML=arr[flag];
	}else{
		alert(flag);
		document.getElementById(aid).innerHTML="?";
	}
}

function changeLogic(lid,aid,tid){
	var flag=document.getElementById(lid).value;
	flag=1-flag;
	document.getElementById(lid).value=flag;
	logi_text(lid,aid,tid);
}
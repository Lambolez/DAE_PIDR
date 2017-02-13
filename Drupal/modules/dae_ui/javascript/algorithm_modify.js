function step(step_num,val,bclr){
	var did="step"+step_num;
	if(bclr){
		clearStepsAfter(step_num);
	}
	$.ajax({
   			type: "GET",
			url: "./?q=algorithm/modify_form/"+step_num+"/"+val,
   			success: function(msg){
				eval(msg);
   			}
 		});
 	
}

function clearStepsAfter(sn){
	var max=6;
	var cur=sn+1;
	while(cur<=max){
		var id="step"+cur;
		document.getElementById(id).innerHTML="";
		cur=cur+1;
	}
}

function getSelect(){
	return document.getElementById("select_aid").value;
}

function add(aid,total,st){
	var sp="@@@";
	var str=""+aid;
	for(i=1;i<=total;i++){
		str+=sp+getLine(i,st,false);
	}
	str+=sp+getLine(-1,st,true);
	step(st,str,false);
}

function edit(aid,cnt,total,st){
	var sp="@@@";
	var str=""+aid;
	for(i=1;i<=total;i++){
		str+=sp+getLine(i,st,(i==cnt));
			
	}
	step(st,str,false);
}

function remove(aid,cnt,total,st){
	var sp="@@@";
	var str=""+aid;
	for(i=1;i<=total;i++){
		if(i!=cnt){
			str+=sp+getLine(i,st,false);
		}
	}
	step(st,str,false);
}

function moveUp(aid,cnt,total,st){
	if(cnt!=1){
		var sp="@@@";
		var str=""+aid;
		for(i=1;i<=total;i++){
			if(i==cnt){
				str+=sp+getLine(i-1,st,false);
			}else if(i==cnt-1){
				str+=sp+getLine(i+1,st,false);
			}else{
				str+=sp+getLine(i,st,false);
			}
		}
		step(st,str,false);
	}
}

function moveDown(aid,cnt,total,st){
	if(cnt!=total){
		var sp="@@@";
		var str=""+aid;
		for(i=1;i<=total;i++){
			if(i==cnt){
				str+=sp+getLine(i+1,st,false);
			}else if(i==cnt+1){
				str+=sp+getLine(i-1,st,false);
			}else{
				str+=sp+getLine(i,st,false);
			}
		}
		step(st,str,false);
	}
}

function getLine(i,st,b){
	var sp2="!!!";
	var line;
	//$flag $sp2 $name $sp2 $des $sp2 $dtid $sp2 $req
	if(i==-1){
		line="1"+sp2+""+sp2+""+sp2+""+sp2+"";
	}else{
		if(b){
			line="1";
		}else{
			line=document.getElementById("flag_"+st+"_"+i).value;
		}
		line+=sp2+document.getElementById("name_"+st+"_"+i).value;
		line+=sp2+document.getElementById("des_"+st+"_"+i).value;
		line+=sp2+document.getElementById("dtid_"+st+"_"+i).value;
		if(document.getElementById("flag_"+st+"_"+i).value=="0"){
			line+=sp2+document.getElementById("req_"+st+"_"+i).value;
		}else{
			if(document.getElementById("req_"+st+"_"+i).checked){
				line+=sp2+"1";
			}else{
				line+=sp2+"0";
			}
		}
	}
	return line;
}

function chooseType(aid,ct,ttl,st){
	var sval=document.getElementById("dtid_"+st+"_"+ct).value;
	if(sval.length==0){
		sval="-1";
	}
	jConfirm("<div id=\"ct_div1\"></div><div id=\"ct_div2\"></div><script> ct_div(1,"+sval+");</script>","choose data type",
		function(r){ 
			type_set(r,aid,ct,ttl,st);
		});
	
}


function type_set(b,aid,ct,total,st){
	var sval=document.getElementById("ct_tmp").value;
	document.getElementById("ct_tmp").value="";
	if(b){
		document.getElementById("dtid_"+st+"_"+ct).value=sval;
		var sp="@@@";
		var str=""+aid;
		for(i=1;i<=total;i++){
			str+=sp+getLine(i,st,false);
		}
		step(st,str,false);
	}
}
function ct_div(p,val){
	document.getElementById("ct_div"+p).innerHTML="loading...";
	$.ajax({
   			type: "GET",
			url: "./?q=algorithm/choose_type/"+p+"/"+val,
   			success: function(msg){
   				eval(msg);
   			}
 		});
 	
}

function ct_save(){
	var name=document.getElementById("ctc_name").value;
	var des=document.getElementById("ctc_des").value;
	var type=document.getElementById("ctc_type").value;
	if(name.length>0&&des.length>0&&type.length>0){
		$.ajax({
   			type: "POST",
			url: "./?q=algorithm/save_type/",
			data: "name="+name+"&des="+des+"&type="+type,
   			success: function(msg){
   				if(msg!="-1"){
   					ct_div(1,msg);
	   				document.getElementById("ct_tmp").value=msg;
	   			}
   			}
 		});
	}else{
		alert("please fill all the fields.");
	}
}

function ct_text_val(){
	var sval=document.getElementById("ct_dtid").value;
	document.getElementById("ct_tmp").value=sval;
	ct_div(2.1,sval);
	ct_div(2.2,sval);
}

function form_check(){
	if(!notEmpty("alg_description","algorithm description")){
		return false;
	}
	if(!notEmpty("alg_version","algorithm version")){
		return false;
	}
	if(!notExistsVersion()){
		return false;
	}
	if(!notEmpty("alg_timecomplexity","algorithm time complexity")){
		return false;
	}
	if(!notEmpty("alg_spacecomplexity","algorithm space complexity")){
		return false;
	}
	if(!notEmpty("alg_file","upload file")){
		return false;
	}
	
	var t3=document.getElementById("total_3").value;
	for(i=1;i<=t3;i++){
		if(!notEmpty("name_3_"+i,"name of Input "+i)){
			return false;
		}
		if(!notEmpty("des_3_"+i,"description of Input "+i)){
			return false;
		}
		if(!notEmpty("dtid_3_"+i,"datatype of Input "+i)){
			return false;
		}
	}
	
	var t4=document.getElementById("total_4").value;
	for(i=1;i<=t4;i++){
		if(!notEmpty("name_4_"+i,"name of Input "+i)){
			return false;
		}
		if(!notEmpty("des_4_"+i,"description of Input "+i)){
			return false;
		}
		if(!notEmpty("dtid_4_"+i,"datatype of Input "+i)){
			return false;
		}
	}
	
	return true;
}

function notEmpty(id,name){
	var obj=document.getElementById(id);
	if(obj!=null){
		if(obj.value.length==0){
			alert("Please fill in: "+name);
			return false;
		}
	}
	return true;
}

function notExistsVersion(){
	var sp="___";
	// unique (name,version)?
	var val=document.getElementById("alg_name").value+sp+document.getElementById("alg_version").value;
	$.ajax({
   			type: "GET",
			url: "./?q=algorithm/modify_form/100"+"/"+val,
   			success: function(msg){
				if(msg>0){
					alert("the algorithm with the same name has  the same version in the database.");
					return false;
				}else{
					return true;
				}
   			}
 		});
 	return true;
}

function wupdate(){
  var xmlhttp=new XMLHttpRequest();
  xmlhttp.onreadystatechange=function()
  {
    if (xmlhttp.readyState==4 && xmlhttp.status==200){
      var obj = JSON.parse(xmlhttp.responseText);
      //alert(obj.B1);
      document.getElementById("p1_in").className="button p_in_"+obj.v_in1;
      document.getElementById("p1_out").className="button p_out_"+obj.v_out1;
	  document.getElementById("p2_in").className="button p_in_"+obj.v_in2;
      document.getElementById("p2_out").className="button p_out_"+obj.v_out2;
      document.getElementById("valve1_1").className="button valve_"+obj.v_s1_1;
      document.getElementById("valve1_2").className="button valve_"+obj.v_s1_2;
	  document.getElementById("valve2_1").className="button valve_"+obj.v_s2_1;
	  document.getElementById("valve2_2").className="button valve_"+obj.v_s2_2;
	  document.getElementById("valve1_3").className="button valve_"+obj.v_s1_3;
	  document.getElementById("valve2_3").className="button valve_"+obj.v_s2_3;
	  document.getElementById("barrel_1").className="barrel_"+obj.B1;
	  document.getElementById("barrel_2").className="barrel_"+obj.B2;
	  document.getElementById("press_1").className="press_"+obj.P1;
	  document.getElementById("press_2").className="press_"+obj.P2;
	  //Geka valves
	  document.getElementById("valve2_4").className="button valve_"+obj.v_s2_4;
	  document.getElementById("valve2_5").className="button valve_"+obj.v_s2_5;
          document.getElementById("valve3_1").className="button valve_"+obj.v_s3_1;
    }
  }
  xmlhttp.open("GET","action.php?action=get_status",true);
  xmlhttp.send();

}

function switch_valve(sender){
  valve_name=sender.id;
  valve_class=sender.className;

  //alert(valve_name+":"+valve_mode);
  var xmlhttp=new XMLHttpRequest();
  xmlhttp.onreadystatechange=function()
  {
    if (xmlhttp.readyState==4 && xmlhttp.status==200){
	setTimeout(wupdate, 500);
    }
  }
  sender.classList.add("pressed");
  xmlhttp.open("GET","action.php?action=switch&valve="+valve_name+"&class="+valve_class,true);
  xmlhttp.send();
}

document.body.onload = wupdate();
setInterval(wupdate, 5000);

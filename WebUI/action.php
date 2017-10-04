<?php
header("Cache-Control: no-store, no-cache, must-revalidate, max-age=0");
header("Cache-Control: post-check=0, pre-check=0", false);
header("Pragma: no-cache");
session_start();

if(!isset($_SESSION["PLC_IP"])) {
	die("Operation is forbidden! User is not logged in!");
}

$error="";
if (isset($_GET["action"])){
	$action = htmlspecialchars($_GET["action"]);
}
if(isset($_GET["valve"])){
	$valve = htmlspecialchars($_GET["valve"]);
}
if(isset($_GET["class"])){
	$class = htmlspecialchars($_GET["class"]);
}

#Checking input data

#
$json = "";
$IP=$_SESSION["PLC_IP"];
#

if ($action == "get_status"){
  $url = "http://$IP/get_status/";
  $json = file_get_contents($url);
  if (!$json){
    die("Arduino is not responding!");
  }
  $obj = json_decode($json);
  if (!$obj){
    die("Wrong json is received!\n$json");
  }
  # Checking barrel 1 
  if ($obj->inputs[0] && $obj->inputs[1]){
    $barrel1_status="middle";
  } else if (!$obj->inputs[0] && $obj->inputs[1]){
    $barrel1_status="high";
  } else if ($obj->inputs[0] && !$obj->inputs[1]){
    $barrel1_status="low";
  } else {
    $barrel1_status="unknown";
  }
  # Checking barrel 2
  if ($obj->inputs[2] && $obj->inputs[3]){
    $barrel2_status="middle";
  } else if (!$obj->inputs[2] && $obj->inputs[3]){
    $barrel2_status="high";
  } else if ($obj->inputs[2] && !$obj->inputs[3]){
    $barrel2_status="low";
  } else {
    $barrel2_status="unknown";
  }
  # Checking Pressure 1
  if (!$obj->inputs[4] && !$obj->inputs[5] && $obj->outputs[1]){
    $press1="norm";
  } else if (!$obj->inputs[4] && !$obj->inputs[5]){
    $press1="broken";
  } else if ($obj->inputs[4] && !$obj->inputs[5]){
    $press1="low";
  } else if (!$obj->inputs[4] && $obj->inputs[5]){
    $press1="high";
  } else if ($obj->inputs[4] && $obj->inputs[5]){
    $press1="short";
  } else {
    $press1="unknown";
  }  
  # Checking Pressure 2
  if (!$obj->inputs[6] && !$obj->inputs[7] && $obj->outputs[3]){
    $press2="norm";
  } else if (!$obj->inputs[6] && !$obj->inputs[7]){
    $press2="broken";
  } else if ($obj->inputs[6] && !$obj->inputs[7]){
    $press2="low";
  } else if (!$obj->inputs[6] && $obj->inputs[7]){
    $press2="high";
  } else if ($obj->inputs[6] && $obj->inputs[7]){
    $press2="short";
  } else {
    $press2="unknown";
  }  
  
  $res=array(
  'P1'=>$press1,
  'P2'=>$press2,
  'B1'=>$barrel1_status,
  'B2'=>$barrel2_status,
  'v_in1'=>$obj->outputs[0],
  'v_out1'=>$obj->outputs[1],
  'v_in2'=>$obj->outputs[2],
  'v_out2'=>$obj->outputs[3],
  'v_s1_1'=>$obj->outputs[4],
  'v_s1_2'=>$obj->outputs[5],
  'v_s1_3'=>$obj->outputs[6],
  'v_s2_1'=>$obj->outputs[7],
  'v_s2_2'=>$obj->outputs[8],
  'v_s2_3'=>$obj->outputs[9],
  'v_s2_4'=>$obj->outputs[10],
  'v_s2_5'=>$obj->outputs[11],
  'v_s3_1'=>$obj->outputs[12]);
  echo json_encode($res);

} else if ($action == "switch"){
  #valve=valve2_3&class=button%20valve_false
  #valve=p1_out&class=button%20p_out_false 
  if ($valve == "p1_in"){
    $port=1;
  } else if ($valve == "p1_out"){
    $port=2;
  } else if ($valve == "p2_in"){
    $port=3;
  } else  if ($valve == "p2_out"){
    $port=4;
  } else if ($valve == "valve1_1"){
    $port=5;
  } else if ($valve == "valve1_2"){
    $port=6;
  } else if ($valve == "valve1_3"){
    $port=7;
  } else if ($valve == "valve2_1"){
    $port=8;
  } else if ($valve == "valve2_2"){
    $port=9;
  } else if ($valve == "valve2_3"){
    $port=10;
  } else if ($valve == "valve2_4"){
    $port=11;
  } else if ($valve == "valve2_5"){
    $port=12;
  } else if ($valve == "valve3_1"){
    $port=13;
  } else {
    die ("Unknown valve name");                   
  }
  if (substr($class, -5) == "false"){
	#print "Port=$port, Mode=$mode\n";      
	$url = "http://$IP/set_value?digital_pin=$port&value=1";
	#print "<br>$url<br>";
	$res = file_get_contents($url); 
  } else if(substr($class, -4) == "true"){
	$url = "http://$IP/set_value?digital_pin=$port&value=0";
	$res = file_get_contents($url); 
  }else{
    die("Unknown class, must be true or false in the end!");
  }        
  
  if (substr($res,0,3) != "OK!"){
    die("Invalid response from PLC: |$res|");
  }          
  print $res;
  
} else if ($action == "get_log"){
  print "get_log";
} else {
  die("Unknown request");
}

?>

<?php
session_start();
if(!isset($_SESSION["user_name"]) || !isset($_SESSION["template"])) {
	header("Location:login.php");
	exit;
}else{
	$templ = file_get_contents($_SESSION["template"].".html");
	print $templ;
	}
?>
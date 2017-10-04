<?php
session_start();
$login_msg="";
if(isset($_POST["user_name"])) {
	$database="/var/spool/watering_db/w_database.dat";
	$db = new SQLite3($database);
	//$_POST["user_name"]='siaffa';
	//$_POST["password"]='*****';
	//print "SELECT username,template FROM users WHERE username='" . $_POST["user_name"] . "' and password = '". $_POST["password"]."' limit 1";
	$statement = $db->prepare('SELECT username,template, PLC_IP FROM users WHERE username=:user_name and password = :password limit 1');
	$statement->bindValue(':user_name', $_POST["user_name"]);
	$statement->bindValue(':password', $_POST["password"]);
	$result = $statement->execute();
	$row = $result->fetchArray();
	//$result = $db->querySingle("SELECT username,template, PLC_IP FROM users WHERE username='" . $_POST["user_name"] . "' and password = '". $_POST["password"]."' limit 1", TRUE);
	//print $result["username"]."\n";
	//print $result["template"]."\n";
	if($row["username"] != "") {
		$_SESSION["user_name"] = $row["username"];
		$_SESSION["template"] = $row["template"];
		$_SESSION["PLC_IP"] = $row["PLC_IP"];
		header("Location:index.php");
		exit;
	} else {
		$login_msg="Не верные логин или пароль!";
	}
}
print '
<html>
    <meta http-equiv="Content-Type" content="text/html;charset=UTF-8">
    <meta name="viewport" content="width = 320, initial-scale = 1, user-scalable = no">
<head>
<title>User Login</title>
<link rel="stylesheet" type="text/css" href="watering.css" />
</head>
<body>
<div class="title">Cloud Watering System</div>
<br>
<form name="frmUser" method="post" action="login.php">
<div class="message">';
print $login_msg;
print '</div>
<table border="0" cellpadding="10" cellspacing="1" width="320" align="center">
<tr class="tableheader">
<td align="center" colspan="2">Введите логин и пароль</td>
</tr>
<tr class="tablerow">
<td align="right">Логин</td>
<td><input type="text" name="user_name"></td>
</tr>
<tr class="tablerow">
<td align="right">Пароль</td>
<td><input type="password" name="password"></td>
</tr>
<tr class="tableheader">
<td align="center" colspan="2"><input type="submit" name="submit" value="Отправить"></td>
</tr>
</table>
</form>
</body></html>
';

?>
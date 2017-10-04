<?php
session_start();
unset($_SESSION["user_name"]);
unset($_SESSION["sitename"]);
unset($_SESSION["template"]);
header("Location:index.php");
?>
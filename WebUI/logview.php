<?php
header("Cache-Control: no-store, no-cache, must-revalidate, max-age=0");
header("Cache-Control: post-check=0, pre-check=0", false);
header("Pragma: no-cache");
session_start();

if(!isset($_SESSION["PLC_IP"])) {
        header("Location:index.php");
	die();
}

$page_size=20;
$IP=$_SESSION["PLC_IP"];
if(isset($_GET["page"])){
        $page = htmlspecialchars($_GET["page"]);
}else $page=1;
if (! is_numeric($page) or $page <1)
    $page=1;
$start_from = ($page-1) * $page_size;


$database="/var/spool/watering_db/w_database.dat";
$db = new SQLite3($database);
$statement = $db->prepare('select date,message from logs where node = :ip order by date desc limit :start, :limit;');
$statement->bindValue(':ip', $IP);
$statement->bindValue(':start', $start_from);
$statement->bindValue(':limit', $page_size);

$result = $statement->execute();

print "
    <html>
    <meta http-equiv='Content-Type' content='text/html;charset=UTF-8'>
    <meta name='viewport' content='width = 320, initial-scale = 1, user-scalable = no'>
    <head>
        <link rel='stylesheet' type='text/css' href='watering.css'>
    </head>
    <body>
        <div class='title'>Журнал</div>";
print "<center>";
print "<table border='1' cellpadding='4'>";
print "<tr> <td bgcolor='#CCCCCC'><strong>Дата</strong></td><td bgcolor='#CCCCCC'><strong>Сообщение</strong></td></tr>";
while ($row = $result->fetchArray()) {
    print "<tr><td>".$row[0]."</td><td>".$row[1]."</td></tr>";
}
print "</table>";
$next_page=$page+1;
$prev_page=$page-1;
print "<table border=0 cellpadding='4'><tr>";
print "<td align='left'><a href='index.php'>Назад</a></td>";
print "<td>&nbsp</td><td>"; 
if ($page>1)
    print "<a href='logview.php?page=".$prev_page."'>".$prev_page."</a>"; 
print " Стр. ";
print "<a href='logview.php?page=".$next_page."'>".$next_page."</a>"; 
print "</td>";
print "</tr></table>";
print "</center>";
print "</body></html>";
?>

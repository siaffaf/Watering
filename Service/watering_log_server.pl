#!/usr/bin/perl -w
use strict;
use IO::Socket;
use Time::Piece;
use DBI;

my($sock, $oldmsg, $newmsg, $hisaddr, $hishost, $MAXLEN, $PORTNO, $port, $ipaddr, $date, $err_msg);
$MAXLEN = 1024;

my %Err = (
110 => "Pressure in E1 is Normal",
120 => "Pressure in E2 is Normal",
111 => "E1 Riched high level -> Switch off the filling valve",
121 => "E2 Riched high level -> Switch off the filling valve",
112 => "E1 Riched low level -> Switch off the pump_1 valve",
122 => "E2 Riched low level -> Switch off the pump_2 valve",
411 => "Pressure in E1 is Low -> Switch off the pump_1 valve",
421 => "Pressure in E2 is Low -> Switch off the pump_2 valve",
412 => "Pressure in E1 is High -> Switch off the pump_1 valve",
422 => "Pressure in E2 is High -> Switch off the pump_2 valve",
413 => "Pressure gauge in E1 is short -> Switch off the pump_1 valve",
423 => "Pressure gauge in E2 is short -> Switch off the pump_2 valve",
510 => "E1 Filling valve switched OFF",
511 => "E1 Filling valve switched ON",
520 => "E1 Pump switched OFF",
521 => "E1 Pump switched ON",
530 => "E2 Filling valve switched OFF",
531 => "E2 Filling valve switched ON",
540 => "E2 Pump switched OFF",
541 => "E2 Pump switched ON",
550 => "S1.1 valve switched OFF",
551 => "S1.1 valve switched ON",
560 => "S1.2 valve switched OFF",
561 => "S1.2 valve switched ON",
570 => "S1.3 valve switched OFF",
571 => "S1.3 valve switched ON",
580 => "S2.1 valve switched OFF",
581 => "S2.1 valve switched ON",
590 => "S2.2 valve switched OFF",
591 => "S2.2 valve switched ON",
"5:0" => "S2.3 valve switched OFF",
"5:1" => "S2.3 valve switched ON",
"5;0" => "Reley 11 switched OFF",
"5;1" => "Reley 11 switched ON",
"5<0" => "Reley 12 switched OFF",
"5<1" => "Reley 12 switched ON",
"5=0" => "Reley 13 switched OFF",
"5=1" => "Reley 13 switched ON",
"5>0" => "Reley 14 switched OFF",
"5>1" => "Reley 14 switched ON",
"5?0" => "Reley 15 switched OFF",
"5?1" => "Reley 15 switched ON",
'5@0' => "Boiler OFF",
'5@1' => "Boiler ON",
"5A0" => "Reley 17 switched OFF",
"5A1" => "reley 17 switched ON",

);

# Analyze arguments
my $num_args = $#ARGV + 1;

if ($num_args != 3) {
  print "\nUsage: watering_log_server.pl portnum logfile db_path \n";
  exit 1;
}

$PORTNO = $ARGV[0];
my $log_file = $ARGV[1];
my $db_path = $ARGV[2];    

$sock = IO::Socket::INET->new(LocalPort => $PORTNO, Proto => 'udp')
    or die "socket: $@";

open LOG_FILE, ">>", $log_file
  or die "Error: Can't open log file: $log_file";
  
LOG_FILE->autoflush(1);

my $dbh = DBI->connect("dbi:SQLite:dbname=$db_path",'','',{AutoCommit=>1,RaiseError=>1,PrintError=>0})
  or die "Error: Can't open database $db_path";

my $sth = $dbh->prepare('insert into logs (node,date,code,message) values (?,?,?,?)');
      
while ($sock->recv($newmsg, $MAXLEN)) {
    ($port, $ipaddr) = sockaddr_in($sock->peername);
    $hishost = inet_ntoa($ipaddr);
    $date = Time::Piece->new->strftime('%Y-%m-%d %H:%M:%S');
    $err_msg = $Err{$newmsg};
    if (!$err_msg){
        $err_msg="Unknown error message";
    }
    print LOG_FILE "$date | ";
    print LOG_FILE "$hishost:$port | ";
    print LOG_FILE "$newmsg - $err_msg\n";
    $sth->execute($hishost,$date,$newmsg,$err_msg);
} 
die "recv: $!";


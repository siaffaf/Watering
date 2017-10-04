#!/bin/bash

service watering_log_server stop
/bin/cp -f watering_log_server.service /etc/systemd/system/
/bin/cp -f watering_log_server.pl /usr/local/bin/
chmod 700 /usr/local/bin/watering_log_server.pl 
chown root:root /usr/local/bin/watering_log_server.pl
service watering_log_server start

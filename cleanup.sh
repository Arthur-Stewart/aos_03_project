#!/bin/bash

# Change this to your netid
netid=klh170130

# Root directory of your project

CONFIGLOCAL=$HOME/03_Project/Config_Files/cleanup.txt

n=0

cat $CONFIGLOCAL | sed -e "s/#.*//" | sed -e "/^\s*$/d" |
(
    read i
    echo $i
	nodes=$( echo $i | awk '{ print $1 }' )
    while [[ $n -lt $nodes ]]
    do
    	read line
    	nodeid=$( echo $line | awk '{ print $1 }' )
        host=$( echo $line | awk '{ print $2 }' )
        port=$( echo $line | awk '{ print $3 }' )
        echo $host
        gnome-terminal -e "ssh -o UserKnownHostsFile=/dev/null -o StrictHostKeyChecking=no $netid@$host.utdallas.edu killall -u $netid" &
        sleep 1

        n=$(( n + 1 ))
    done
   
)


echo "Cleanup complete"


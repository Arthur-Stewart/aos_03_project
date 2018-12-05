#!/bin/bash

netid=klh170130

# Root directory of your project
PROJDIR=03_Project

CONFIGLOCAL=$HOME/Computer_Science/Courses/UTD/Advanced_Operating_Systems/Homework/$PROJDIR/Config_Files/config.txt

CONFIGREMOTE=$PROJDIR/Config_Files/six.txt 

# Your executable binary 
PROG=main

n=0

cat $CONFIGLOCAL | sed -e "s/#.*//" | sed -e "/^\s*$/d" |
(
    read i
	nodes=$( echo $i | awk '{ print $1 }' )
    #echo $nodes
    while [[ $n -lt $nodes ]]
    do
    	read line
    	nodeid=$( echo $line | awk '{ print $1 }' )
        host=$( echo $line | awk '{ print $2 }' )
        port=$( echo $line | awk '{ print $3 }' )

		#echo $nodeid
		#echo $host.utdallas.edu
		#echo $port
	
	gnome-terminal -e "ssh -o UserKnownHostsFile=/dev/null -o StrictHostKeyChecking=no $netid@$host.utdallas.edu ./$PROJDIR/$PROG $CONFIGREMOTE $nodeid; exec bash" &

        n=$(( n + 1 ))
    done
)

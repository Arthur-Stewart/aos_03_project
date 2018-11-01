#!/bin/bash

# Change this to your netid
netid=acs170004

# Root directory of your project

CONFIGLOCAL=$HOME/Computer_Science/Courses/UTD/Advanced_Operating_Systems/Homework/02_Project/Config_Files/config.txt

n=0

cat $CONFIGLOCAL | sed -e "s/#.*//" | sed -e "/^\s*$/d" |
(
    read i
    echo $i
	nodes=$( echo $i | awk '{ print $1 }' )
    while [[ $n -lt $nodes ]]
    do
    	read line

        host=$( echo $line | awk '{ print $1 }' )
        echo $host
        urxvt -e sh -c "ssh -o UserKnownHostsFile=/dev/null -o StrictHostKeyChecking=no $netid@$host.utdallas.edu killall -u $netid" &
        sleep 1

        n=$(( n + 1 ))
    done
   
)


echo "Cleanup complete"

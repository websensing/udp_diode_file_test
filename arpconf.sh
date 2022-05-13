#!/bin/bash
# Script to setup arp cache -- must be run as root
if [ $# == 1 ] && [ $1 == "-h" ] ; then
		echo "usage: ./arpconf.sh -- must be run on 'in' or 'out'"
		exit
fi
HOST=${HOSTNAME%%.*}
case ${HOST} in
		in)
				echo "[in]"
				# clear the arp cache
				#arp -d -a
				# remote machine is oout
				#sudo arp -s 172.16.253.1 00:30:18:0e:27:49
				sudo arp -s 172.16.253.1 00:30:18:0e:27:81 
				;;
		out)
				echo "[out]"
				# clear the arp cache
				#arp -d -a
				# remote machine is in
				#sudo arp -s 172.16.253.2 00:30:18:0e:27:63
				sudo arp -s 172.16.253.2 00:30:18:0e:27:83
				;;
		*)
				echo "[unknown host]"
				;;
esac

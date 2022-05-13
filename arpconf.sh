#!/bin/bash

# Script to setup arp cache -- must be run as root
if [ $# == 1 ] && [ $1 == "-h" ] ; then
		echo "usage: ./arpconf.sh in  <outside ip> <outside mac>"
		echo "usage: ./arpconf.sh out <inside ip> <inside mac>"
		exit
fi

HOST=$1

case ${HOST} in
		in)
				echo "[in] --> [out] @ $2 $3"
				sudo arp -s $2 $3
				;;
		out)
				echo "[out] <-- [in] @ $2 $3"
				sudo arp -s $2 $3
				;;
		*)
				echo "[unknown host]"
		    echo "usage: ./arpconf.sh in  <outside ip> <outside mac>"
		    echo "usage: ./arpconf.sh out <inside ip> <inside mac>"
		    ;;
esac

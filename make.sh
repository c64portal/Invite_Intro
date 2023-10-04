#!/bin/bash


T=`date +"%Y%m%d_%H%M%S"`


KICK=~/c64/kickc085

export DIR=`pwd`

if [ -z "$1" ]
	then
		echo
		echo "Available arguments: main, "
		echo
	else
		if [ $1 = "main" ]
		then
			$KICK/bin/kickc.sh -Warraytype main.c -S $2
		fi


fi





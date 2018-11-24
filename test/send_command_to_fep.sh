#!/bin/bash

if [ -z "$1" ]
then
	echo "NU AI SPECIFICAT FOLDERUL! Ex: serial, io, etc"
	exit 1
fi
if [ -z "$2" ]
then
	echo "NU AI SPECIFICAT BRANCH-ul"
	exit 1
fi

ssh fep gaussian-filter/test/bin/run_on_fep.sh $1 $2


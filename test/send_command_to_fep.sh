#!/bin/bash

if [ -z "$1" ]
then
	echo "NU AI SPECIFICAT FOLDERUL! Ex: serial, io, etc"
	exit 1
fi
if [ -z "$2" ]
then
	echo "Se foloseste branch-ul default de pe fep"
fi

ssh fep gaussian-filter/test/bin/run_on_fep.sh $1 $2


#!/bin/bash

cd ~/gaussian-filter
git reset --hard HEAD
git pull

FOLDER=$1

cd $FOLDER
make compile

qsub -cwd -sync y -e ~/gaussian-filter/test/output/out1.e -o ~/gaussian-filter/test/output/out1.o -q ibm-dp.q ~/gaussian-filter/test/bin/run.sh $FOLDER

diff ../test/output/out1.ppm ../test/ref/ref1.ppm
if [ $? -eq 0 ]; then
	cat ../test/output/out1.e
	cat ../test/output/out1.o
else
	echo "THE OUTPUT IS DIFFERENT FROM THE REFFERENCE!"
fi

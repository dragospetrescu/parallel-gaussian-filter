#!/bin/bash

cd ~/gaussian-filter
git reset --hard HEAD
git pull

FOLDER=$1

cd $FOLDER
make compile

qsub -sync y -e ../test/output/out1.e -o ../test/output/out1.o -q ibm-dp.q time ./blur ../test/input/in1.ppm ../test/output/out1.ppm 15 3

diff ../test/output/out1.ppm ../test/ref/ref1.ppm
if [ $? -eq 0 ]; then
	cat ../test/output/out1.e
	cat ../test/output/out1.o
else
	echo "THE OUTPUT IS DIFFERENT FROM THE REFFERENCE!"
fi

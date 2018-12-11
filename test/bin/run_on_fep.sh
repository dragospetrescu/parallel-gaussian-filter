#!/bin/bash

cd ~/gaussian-filter
git reset --hard HEAD
if [ -n "$2" ]; then
	git checkout $2
fi
git pull

FOLDER=$1
module load libraries/openmpi-2.0.1-gcc-5.4.0
cd $FOLDER
make compile

mkdir ~/gaussian-filter/test/output/ &> /dev/null
qsub -sync y -o ~/gaussian-filter/test/output/out1.o -e ~/gaussian-filter/test/output/out1.e -q ibm-dp.q ~/gaussian-filter/test/bin/run.sh $FOLDER 1
qsub -sync y -o ~/gaussian-filter/test/output/out1.o -e ~/gaussian-filter/test/output/out1.e -q ibm-dp.q ~/gaussian-filter/test/bin/run.sh $FOLDER 2
qsub -sync y -o ~/gaussian-filter/test/output/out1.o -e ~/gaussian-filter/test/output/out1.e -q ibm-dp.q ~/gaussian-filter/test/bin/run.sh $FOLDER 4
qsub -sync y -o ~/gaussian-filter/test/output/out1.o -e ~/gaussian-filter/test/output/out1.e -q ibm-dp.q ~/gaussian-filter/test/bin/run.sh $FOLDER 8
qsub -sync y -o ~/gaussian-filter/test/output/out1.o -e ~/gaussian-filter/test/output/out1.e -q ibm-dp.q ~/gaussian-filter/test/bin/run.sh $FOLDER 16
qsub -sync y -o ~/gaussian-filter/test/output/out1.o -e ~/gaussian-filter/test/output/out1.e -q ibm-dp.q ~/gaussian-filter/test/bin/run.sh $FOLDER 32


#diff ../test/output/out1.ppm ../test/ref/ref1.ppm
#if [ $? -eq 0 ]; then
#	echo "OUTPUTS ARE IDENTICAL!"
#	cat ../test/output/out1.e
#	cat ../test/output/out1.o
#else	echo "THE OUTPUT IS DIFFERENT FROM THE REFFERENCE!"
#fi

#mv ~/gaussian-filter/test/output/out1.o ~/gaussian-filter/test/output/out1.o_bkp &> /dev/null
#mv ~/gaussian-filter/test/output/out1.e ~/gaussian-filter/test/output/out1.e_bkp &> /dev/null


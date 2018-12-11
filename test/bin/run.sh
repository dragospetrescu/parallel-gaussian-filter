#!/bin/bash


module load compilers/solarisstudio-12.5
module load libraries/openmpi-2.0.1-gcc-5.4.0
cd
if [ $1 = "openmp" ] || [ $1 = "openmp+io" ]; then
   export OMP_NUM_THREADS=$2
elif [ $1 = "pthreads" ]; then
    sed -i "s/#define NUM_THREADS [0-9.]\+/#define NUM_THREADS ${2}/" gaussian-filter/pthreads/src/filter.h
    make -C gaussian-filter/pthreads clean
    make -C gaussian-filter/pthreads
fi


if [ $1 = "mpi" ]; then
    collect -p hi mpirun -np $2 ./gaussian-filter/$1/blur gaussian-filter/test/input/in2.ppm gaussian-filter/test/output/out2.ppm 15 3
else
    collect -p hi ./gaussian-filter/$1/blur gaussian-filter/test/input/in1.ppm gaussian-filter/test/output/out1.ppm 15 3
fi
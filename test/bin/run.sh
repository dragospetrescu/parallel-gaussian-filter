#!/bin/bash


module load compilers/solarisstudio-12.5
cd
if [ $1 = "openmp+io" ]; then
   export OMP_NUM_THREADS=$2
else
    sed -i "s/#define NUM_THREADS [0-9.]\+/#define NUM_THREADS ${2}/" gaussian-filter/pthreads/src/filter.h
    make -C gaussian-filter/pthreads clean
    make -C gaussian-filter/pthreads

fi

collect -p hi ./gaussian-filter/$1/blur gaussian-filter/test/input/in1.ppm gaussian-filter/test/output/out1.ppm 15 3

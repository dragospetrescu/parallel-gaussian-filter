#!/bin/bash

export OMP_NUM_THREADS=$2
module load compilers/solarisstudio-12.5
cd
collect ./gaussian-filter/$1/blur gaussian-filter/test/input/in1.ppm gaussian-filter/test/output/out1.ppm 15 3

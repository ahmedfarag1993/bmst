#!/bin/bash

for OPT in noopt O2 O3
do
   for NPROC in 1 2 4 8 16 32
   do
      llsubmit "bmst_${NPROC}_${OPT}.job"
   done
done

#!/bin/bash

folder=$1
num=`cat $folder/bnumber`
let num++
echo "$num" | tee $folder/bnumber #<-- output and save the number back to file
dat=`date +%d-%m-%Y\ %H:%M:%S`
out="$num $dat"
echo '#define BUILD "BUILD #'$out'"' | tee $folder/bnumber.h

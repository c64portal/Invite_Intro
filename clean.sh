#!/bin/bash
#
# 
#
#

PWD=`pwd`

echo
echo "Cleaning source dir "

rm -rf *.bin
rm -rf *.d64
rm -rf *.prg
rm -rf *.b2
rm -rf *.sym
rm -rf  main.asm

rm -rf *.klog
rm -rf *.vs
rm -rf *.dbg

rm -rf bin/*
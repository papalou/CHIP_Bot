#!/bin/bash
export ARM_TOOLCHAIN_PATH=$(pwd)/../toolchain/
export ARM_TOOLCHAIN_PRFIX=arm-linux-gnueabihf-

export LD_LIBRARY_PATH=$ARM_TOOLCHAIN_PATH/lib
export PATH=$ARM_TOOLCHAIN_PATH/bin:$PATH

if [ $# -eq 0 ] ; then echo ; echo " ---==== $(basename $(pwd))  ====---"; echo ; fi
make --no-print-directory  ARCH=arm CROSS_COMPILE=$ARM_TOOLCHAIN_PRFIX SYSROOT=$(pwd)/../buildroot/output/host/  -j2 $1

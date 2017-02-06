#!/bin/bash
export ARM_TOOLCHAIN_PATH=$(pwd)/../../toolchain/
export ARM_TOOLCHAIN_PRFIX=arm-buildroot-linux-uclibcgnueabihf-

export LD_LIBRARY_PATH=$ARM_TOOLCHAIN_PATH/usr/lib
export PATH=$ARM_TOOLCHAIN_PATH/usr/bin:$PATH

if [ $# -eq 0 ] ; then echo ; echo " ---==== $(basename $(pwd))  ====---"; echo ; fi
make --no-print-directory  ARCH=arm CROSS_COMPILE=$ARM_TOOLCHAIN_PRFIX SYSROOT=$(pwd)/../../buildroot/output/host/  $1

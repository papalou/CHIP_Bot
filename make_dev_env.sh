#!/bin/sh

if [ ! -e buildroot ]; then
	echo "Clone buildroot"
	git clone https://github.com/papalou/buildroot 
fi

if [ ! -e libcommon ]; then
	echo "Clone needed lib"
	git clone https://github.com/papalou/libcommon
fi

if [ ! -e linux ]; then
	echo "Clone Linux Kernel"
	git clone -b CHIP_PAPALOU https://github.com/papalou/CHIP-linux linux
fi

if [ ! -e uboot ]; then
	echo "Clone UBoot"
	git clone https://github.com/papalou/CHIP-u-boot uboot
fi

if [ ! -e toolchain ]; then
	echo "Install toolchain"
	tar xf archives/gcc-linaro-6.2.1-2016.11-x86_64_arm-linux-gnueabihf.tar.xz
	mv gcc-linaro-6.2.1-2016.11-x86_64_arm-linux-gnueabihf toolchain
fi

echo "Ready to go"

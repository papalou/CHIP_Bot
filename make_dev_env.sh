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
	git clone -b CHIP_PAPALOU https://github.com/papalou/CHIP-u-boot uboot
fi

if [ ! -e toolchain ]; then
	echo "Install toolchain"
	tar xf archives/gcc-linaro-6.2.1-2016.11-x86_64_arm-linux-gnueabihf.tar.xz
	mv gcc-linaro-6.2.1-2016.11-x86_64_arm-linux-gnueabihf toolchain
fi

#Clone needed (host) tools
if [ ! -e tools ]; then
	echo "Create tools folder"
	mkdir tools
fi

if [ ! -e tools/sunxi-tools ]; then
	echo "Clone Sunxi-tools"
	git clone -b origin/master https://github.com/papalou/sunxi-tools tools/sunxi-tools
fi

if [ ! -e tools/chip-mtd-utils ]; then
	echo "Clone Sunxi-tools"
	git clone -b origin/by/1.5.2/next-mlc-debian https://github.com/papalou/CHIP-mtd-utils tools/chip-mtd-utils
fi

#if [ ! -e tools/chip-tools ]; then
#	echo "Clone CHIP-tools"
#	git clone https://github.com/papalou/CHIP-tools tools/chip-tools
#fi

echo "Ready to go"

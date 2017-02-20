#!/bin/sh

if [ ! -e buildroot ]; then
	echo "Clone buildroot"
	git clone -b dev_chipbot https://github.com/papalou/CHIP-buildroot buildroot
fi

if [ ! -e libcommon ]; then
	echo "Clone needed lib"
	git clone https://github.com/papalou/libcommon
fi

if [ ! -e toolchain ]; then
	echo "Install toolchain"
	tar xf archives/gcc-linaro-arm-linux-gnueabihf-4.9-2014.09_linux.tar.xz
	mv gcc-linaro-arm-linux-gnueabihf-4.9-2014.09_linux toolchain
fi

#Clone needed (host) tools
if [ ! -e tools ]; then
	echo "Create tools folder"
	mkdir tools
fi

if [ ! -e tools/sunxi-tools ]; then
	echo "Clone Sunxi-tools"
	git clone -b master https://github.com/papalou/sunxi-tools tools/sunxi-tools
fi

if [ ! -e tools/chip-mtd-utils ]; then
	echo "Clone Sunxi-tools"
	git clone -b by/1.5.2/next-mlc-debian https://github.com/papalou/CHIP-mtd-utils tools/chip-mtd-utils
fi

echo "Ready to go"

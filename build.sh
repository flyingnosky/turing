#!/bin/bash
export PATH=$PATH:/home/chenxiang/toolchains/gcc-linaro-aarch64-linux-gnu-4.9-2014.05_linux/bin/
export ARCH=arm64
export CROSS_COMPILE=aarch64-linux-gnu-

if [ x"$1" = x"4.1" ]; then
	git checkout sas-dev-4.1
	make clean
	make d02_defconfig
else
	git checkout sas-dev
	make clean
	make defconfig
fi

make -j16 Image

if [ x"$2" = x"d02" ]; then
	make hisilicon/hip05-d02.dtb
	cp arch/arm64/boot/dts/hisilicon/hip05-d02.dtb /home/chenxiang/target/
else
	make hisilicon/hisi_p660_evb.dtb
	cp arch/arm64/boot/dts/hisilicon/hisi_p660_evb.dtb /home/chenxiang/target/
if

cp arch/arm64/boot/Image /home/chenxiang/target/


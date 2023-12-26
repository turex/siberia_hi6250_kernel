#!/bin/bash

#
# This Script is by Iceows - build phenix kernel (4.9)
# use linaro 7.5 toolchain
#
# 
# Download linearo toolschain > 4.9, for example
#	https://releases.linaro.org/components/toolchain/binaries/7.5-2019.12/aarch64-elf/
# Extract to 
#	~/gcc/
#
#

rm -rf out/arch/arm64/boot/Image.gz
export PATH=$PATH:~/gcc/gcc-linaro-7.5.0-2019.12-x86_64_aarch64-elf/bin
export CROSS_COMPILE=aarch64-elf-

export GCC_COLORS=auto
export ARCH=arm64

if [ ! -d "out" ];
then
	mkdir out
fi
	

#make ARCH=arm64 distclean
make ARCH=arm64 O=../out siberia_defconfig
make ARCH=arm64 O=../out -j6

rm -rf Siberia_Kirin65x-4.4*.img

./mkbootimg --kernel out/arch/arm64/boot/Image.gz --base 0x00478000 --cmdline "loglevel=4 coherent_pool=512K page_tracker=on slub_min_objects=12 unmovable_isolate1=2:192M,3:224M,4:256M printktimer=0xfff0a000,0x534,0x538 androidboot.selinux=enforcing buildvariant=user" --kernel_offset 0x00008000 --ramdisk_offset 0x07b88000 --second_offset 0x00e88000 --tags_offset 0x07988000 --os_version 8 --os_patch_level 2020-07-01 --output Siberia_Kirin65x-$ver.img



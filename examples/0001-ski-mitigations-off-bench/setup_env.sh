#!/bin/sh

mkdir env && cd env

# TODO: accept the choice of compiling from src and arch selection
cp ../bin/arch/x86_64/bski .

cp ../bin/vmlinux .
cp ../bin/ski-bootloader .
cp ../8192mib-ext4-es-current.img .

ln -s 8192mib-ext4-es-current.img sda

mkdir mnt
mount -o loop sda mnt
patch -p0 mnt/init < ../patches/0001-add-n-benchmarks-to-init-new-upstream.diff
umount mnt

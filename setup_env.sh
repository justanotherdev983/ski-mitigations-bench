#!/bin/sh

mkdir env && cd env

# TODO: accept the choice of compiling from src
cp ../bin/vmlinux .
cp ../bin/bski .
cp ../bin/ski-bootloader .
cp ../8192mib-ext4-es-current.img .

ln -s 8192mib-ext4-es-current.img sda

mkdir mnt
mount -o loop sda mnt
patch -p0 mnt/init < ../patches/0001-add-n-benchmarks-to-init.diff
umount mnt

#!/bin/bash

set -e

echo "Compiling kernel"
make

echo "Updating iso root"
mkdir -p iso_root/boot/
cp -v bin/raios iso_root/boot

echo "Generating bootable ISO"
xorriso -as mkisofs -R -r -J \
	-b boot/limine/limine-bios-cd.bin \
	-no-emul-boot -boot-load-size 4 -boot-info-table \
	-hfsplus -apm-block-size 2048 \
	--efi-boot boot/limine/limine-uefi-cd.bin \
	-efi-boot-part --efi-boot-image \
	--protective-msdos-label \
	iso_root -o image.iso

echo "Done"

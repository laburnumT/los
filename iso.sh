#!/bin/sh
set -e
. ./build.sh

mkdir -p isodir
mkdir -p isodir/boot
mkdir -p isodir/boot/grub

cp sysroot/boot/los.kernel isodir/boot/los.kernel
cat > isodir/boot/grub/grub.cfg << EOF
menuentry "los" {
	multiboot /boot/los.kernel
}
EOF
grub-mkrescue -o los.iso isodir

#!/bin/sh
set -e

mkdir -p isodir
mkdir -p isodir/boot
mkdir -p isodir/boot/grub

cp kernel/los.kernel isodir/boot/los.kernel
cat > isodir/boot/grub/grub.cfg << EOF
menuentry "los" {
	multiboot /boot/los.kernel cmdline_shit
}
EOF
grub-mkrescue -o los.iso isodir

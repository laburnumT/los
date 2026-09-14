#!/bin/sh
set -e

mkdir -p isodir
mkdir -p isodir/boot
mkdir -p isodir/boot/grub

cp kernel/los.kernel isodir/boot/los.kernel
cp root.romfs isodir/boot/root.romfs
cat > isodir/boot/grub/grub.cfg << EOF
menuentry "los" {
	multiboot /boot/los.kernel /bin/lsh
        module /boot/root.romfs root.romfs
}
EOF
grub-mkrescue -o los.iso isodir

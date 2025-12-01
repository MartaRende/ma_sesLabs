#!/bin/sh

# Replace /dev/sdX3 with the correct device node
cryptsetup open --key-file /etc/luks/passphrase /dev/sda3 rootfs-luks

mkdir -p /mnt/luks_rootfs

mount -t ext4 /dev/mapper/rootfs-luks /mnt/luks_rootfs

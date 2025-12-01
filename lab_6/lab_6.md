# Lab 6

## Setup

First, setup cryptsetup in your buildroot config :

```bash
make menuconfig
-> Target Packages
--> Hardware handling
---> [*] cryptsetup
--> Miscellanous
---> [*] haveged


make linux-menuconfig
Go to: device driver → <*> Multiple Devices drivers support (RAID and LVM) 
      → <*>Device mapper support
      → <*> Crypt target support-> Device Driver 
--> <*> Multiple Devices drivers support (RAID and LVM) 
---> <*>Device mapper support
---> <*> Crypt target support
```

> Pay attentions that the device mapper support is enabled in the kernel directly, and not as a module


## Question 1

To generate the passphrase file : 
```bash
dd if=/dev/urandom of=passphrase bs=1 count=64
```


We need to create the encrypted partition on the sd card now :

```bash
sudo fdisk /dev/sda
n
3
[Keep default]
+512M
```

Now that the partition exists, we need to initialize it with LUKS : 
```bash
cryptsetup luksFormat --type luks1 --pbkdf pbkdf2 --key-file passphrase /dev/sda3
```

Now we can open it, and format the new logical volume to ext4 :

```bash
cryptsetup open --key-file passphrase /dev/sdX3 rootfs-luks
mkfs.ext4 /dev/mapper/rootfs-luks
```

Now, we need to copy the rootfs partition to the encrypted one :

```bash
mkdir -p /mnt/buildroot_rootfs
mkdir -p /mnt/luks_rootfs
mount /buildroot/output/images/rootfs.ext4 /mnt/buildroot_rootfs
mount /dev/mapper/rootfs-luks /mnt/luks_rootfs
# The -a allows to keep all permissions and file ownership
rsync -a /mnt/buildroot_rootfs/ /mnt/luks_rootfs/

umount /mnt/buildroot_rootfs
umount /mnt/luks_rootfs
cryptsetup close rootfs-luks
```


Now, to speed up the process, the only way to go is to reduce the complexity of the decryption algorithm used/reduce the number of iterations. For example, we could use : 
```bash
cryptsetup luksFormat --type luks1 --pbkdf pbkdf2 --pbkdf-force-iterations 1000 --key-file passphrase /dev/sdX3
```

In order for the script to automount the partition, we need to save the passphrase on the disk. 

So to prepare for this step, put the passphrase file under `/etc/luks/passphrase` with strict permissions such as :
```bash
chmod 400 /etc/luks/passphrase
```

And then, put the `luks_automount.sh` script in the rootfs overlay `<board_path>/rootfs_overlay/etc/S40luks`


## Question 2

To create the initramfs "by hand", we can follow the instruction given in the slides, we have merged them in this file : `./build_initramfs.sh`
This script needs to be launched inside the docker environment.

Once this script ran, do not forget to edit the `<board>/boot.cmd` and add those two lines : 
```bash
ext4load mmc 0 0x50000000 uInitrd
booti $kernel_addr_r 0x50000000 $fdt_addr_r
```

Once flashed and booted, you need to unmount the standard rootfs partition using : 
```bash
umount /newroot/dev
umount /newroot
rmdir /newroot
```

Then, you need to decrypt the encrypted partition, and mount it using :
```bash
cryptsetup open --type luks1 --key-file /passphrase /dev/mmcblk0p3 rootfs-luks
mkdir /newroot
mount /dev/mapper/rootfs-luks /newroot
mount -n -t devtmpfs devtmpfs /newroot/dev
```

You can then run the switch-root :

```bash
exec switch_root /newroot /sbin/init
```

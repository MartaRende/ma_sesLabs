# Rapport - Lab 2

# Part 1

## Question 1


La première demande consiste à modifier la configuration buildroot afin de personnaliser  l'expérience et de distinguer notre U-Boot de celui par défaut. Pour ce faire, il est nécessaire de créer un fichier fragment contenant les options qu'on souhaite personnaliser. Dans notre cas, le démarrage par défaut, qui doit normalement prendre 2 secondes, doit prendre 10 secondes, et le prompt par défaut « => » doit être remplacé par « NanoPi # ».

Pour ce faire, en s'assure d'être dans le bon dossier

```bash
cd /buildroot2
```

Ensuite nous allons dans le dossier:

```bash
cd board/friendlyarm/nanopi-neo-plus2
```
Ici on va créer le fragment file

```bash
touch uboot-extras.config
```
Après on peux ajouter dedans le fichier les options qu'on souhaite modifier :

```bash
nano uboot-extras.config
```

On copie colle dedans ses 2 options:

```bash
CONFIG_BOOTDELAY=10
CONFIG_SYS_PROMPT="NanoPi # "
```

Maintenant il faut ajouter dedans la config buildroot le fragment file. Ainsi, lorsque nous passerons à la compilation, Buildroot saura qu'il doit prendre en compte notre fichier fragment et modifier la configuration en fonction de ce qui y est écrit.

Pour faire ça il faut faire :

```bash
make menuconfig
```

Puis aller:

```bash
Bootloaders  --->
    U-Boot  --->
        Additional configuration fragment files = board/friendlyarm/nanopi-neo-plus2/uboot-extras.config
```

Sauver les modifications faites dans le menuconfig

Pour re-compiler l'image, soit nettoyer que u-boot en faisant :
```bash
rm -r ./output/build/uboot-2020.10-rc5
```

Ou alors nettoyer buildroot en entier :

```bash
make clean
```

Ensuite :

```bash
make -j <job_n>
```

Une fois que c'est fait, on peut contrôler que nos modifications ont bien été prises en compte avec par exemple :
```bash
cat ouput/build/uboot-2020.10-rc5/.config | grep CONFIG_SYS_PROMPT
#Should ouput : CONFIG_SYS_PROMPT="NanoPi # "
```

Enfin il faut flash l'image sur la SD donc :

```bash
sudo dd if= /output/images/sdcard.img of=/dev/sda bs=1M
sudo sync
```

## Question 2

L'idée de cette partie est de transitionner à un fichier FIT (flattened image tree).

En premier, il faut créer le fichier .its (image tree source) :

```bash
cd ./output/image/
touch kernel_fdt.its
```

Ensuite, on peut ajouter ce contenu :

`./output/image/kernel_fdt.its`
```
/dts-v1/;

/ {
    description = "FIT image with kernel and FDT";
    #address-cells = <1>;

    images {
        kernel {
            description = "Linux Kernel";
            data = /incbin/("Image");
            type = "kernel";
            arch = "arm64";
            os = "linux";
            compression = "none";
            load = <0x40080000>;
            entry = <0x40080000>;
            hash {
                algo = "sha256";
            };
        };

        fdt {
            description = "Flattened Device Tree";
            data = /incbin/("sun50i-h5-nanopi-neo-plus2.dtb");
            type = "flat_dt";
            arch = "arm64";
            compression = "none";
            load = <0x4FA00000>;
            hash {
                algo = "sha256";
            };
        };
    };

    configurations {
        default = "conf";

        conf {
            kernel = "kernel";
            fdt = "fdt";
        };
    };
};
```

Ensuite, nous pouvons compiler ce fichier en un fichier .itb en faisant :

```bash
mkimage -f kernel_fdt.its kernel_fdt.itb
```

Si il n'y a pas d'erreurs, on doit obtenir une sortie ressemblant à :
```
FIT description: FIT image with kernel and FDT for NanoPi NEO Plus2
Created:         Thu Oct  3 14:23:01 2025
 Image 0 (kernel)
  Description:  Linux Kernel
  Type:         Kernel Image
  ...
 Image 1 (fdt)
  Description:  Device Tree Blob
```

Ensuite, il faut modifier le fichier `board/friendlyarm/nanopi-neo-plus2/genimage.cfg`, car la génération de l'image pour la carte SD a changé (dans la partition FAT, on a plus que )

Il faut donc remplacer cette partie :
`board/friendlyarm/nanopi-neo-plus2/genimage.cfg` (old)
```
image boot.vfat {
    vfat {
        files = {
            "boot.scr",
            "Image",
            "sun50i-h5-nanopi-neo-plus2.dtb"
        }
    }
    size = 64M
}
```

Par celle là :
`board/friendlyarm/nanopi-neo-plus2/genimage.cfg` (new)
```
image boot.vfat {
        vfat {
                files = {
                        "boot.scr",
                        "kernel_fdt.itb"
                }
        }

        size = 64M
}
```

Ensuite, il suffit de relancer :
```bash
make -j<job_n>
```

Afin de re-générer l'image de la carte SD.

Il faut donc flasher la carte SD avec la nouvelle image que l'on vient de générer :
```bash
sudo dd if=./output/image/sdcard.img of=/dev/sdcard
```

Pour vérifier que nos changements sont corrects, on peut mount la carte SD

```bash
sudo mount /dev/sda2 /mtn
```

Cette partitions ne devrait contenir que deux fichiers : boot.scr  kernel_fdt.itb

```bash
ls /mnt
# Should ouptut ONLY : boot.scr  kernel_fdt.itb
```

Ne pas oublier de unmount la carte SD avant de la retirer :

```bash
sudo umount /mnt
```


Une autre façon de vérifier est de démarrer le nanopi, accéder au prompt U-Boot (appuyer sur une touche pendant la séquence de démarrage), puis lancer la commande :

```bash
fatls mmc 0:1
```

Qui liste les fichiers présents sur la partition 1 (partition FAT de démarrage), elle devrait retourner :

```
279   boot.scr
39499960   kernel_fdt.itb
```

Ensuite, toujours depuis la console U-Boot, pour load l'image FIT en mémoire, lancer la commande :
```bash
fatload mmc 0:1 0x50000000 kernel_fdt.itb
```

On peut ensuite inspecter l'image chargée en mémoire avec :
```bash
iminfo 0x50000000
```

Cette commande retourne beaucoup d'informations, mais notamment, deux images (`Image 0 (Kernel)` et `Image 1 (fdt)`) ainsi que leur hash, dans mon cas :

Pour le kernel :
```
Hash algo:    sha256
Hash value:   a2cc46e7283930ddb5c12782cd93b71aafb907ec84835f377080fefeb0bf2aee
```


Pour le FDT
```
Hash algo:    sha256
Hash value:   c079e21fe64092c5f63a99a25269484890507c90e907c0c1316929a7e1a3a496
```


On peut vérifier que ces hashs sont corrects en faisant (sur la machine Host):

```bash
cd output/image
sha256sum Image
# Output : a2cc46e7283930ddb5c12782cd93b71aafb907ec84835f377080fefeb0bf2aee  Image
sha256sum sun50i-h5-nanopi-neo-plus2.dtb
# Output : c079e21fe64092c5f63a99a25269484890507c90e907c0c1316929a7e1a3a496  sun50i-h5-nanopi-neo-plus2.dtb
```


Ensuite, (de retour sur le Pi, dans la console U-Boot), on peut lancer :

```bash
bootm 0x50000000
```

Afin de boot l'image. Et comme prévu, on a l'erreur : `Image too large: increase CONFIG_SYS_BOOTM_LEN`.

Cette erreur nous indique que le buffer alloué par U-Boot en mémoire est trop petit pour notre FIT. Il faudrait donc rebuild U-Boot avec un `CONFIG_SYS_BOOTM_LEN` agrandi.

## Question 3

Le fichier `sunxi-common.h` contient la ligne :

```
#define CONFIG_SYS_BOOTM_LEN		(32 << 20)
```

Qui défini donc la taille maximum de l'image bootée par la commande bootm à 32M. Or, notre image fait ~37M, il faut donc agrandir cette limite.

On peut donc modifier cette ligne pour remplacer la valeure 32 par 64 (pour passer à une image maximum de 64M).

Cela nous donne donc cette ligne :
```
#define CONFIG_SYS_BOOTM_LEN		(64 << 20)
```

Le fichier patch pour appliquer ces modifications est [ici](./0001-uboot-image-size-fix.patch), et il est à placer dans le dossier : `./board/friendlyarm/nanopi-neo-plus2/patches/uboot/`.

## Question 4

Pour désactiver l'enregistrement de l'environment par u-boot, on peut ajouter cette ligne au fichier de configuration u-boot `./board/friendlyarm/nanopi-neo-plus2/uboot-extras.config` :
```
CONFIG_BOOTDELAY=10
CONFIG_SYS_PROMPT="NanoPi # "
CONFIG_ENV_IS_NOWHERE=y # <---- Add this line
```

Pour que le script `boot.cmd` puisse lire d'une partition ext4 (et non plus VFAT), il faut enlever les lignes commençant par `fatload`, et les remplacer par cette ligne :
```
ext4load mmc 0:1 0x50000000 kernel_fdt.itb
```

Ensuite, nous devons modifier le fichier `genimage.cfg` afin de retirer la création de l'image boot FVAT. Nous devons donc supprimer les lignes :
```
image boot.vfat {
  vfat {
  	files = {
  		"boot.scr",
  		"kernel_fdt.itb"
  	}
  }
  size = 64M
}
```

Et, changer la partie :
```
partition boot {
	partition-type = 0xC
	bootable = "true"
	# image = "boot.vfat" # <- Change this line
	image = "boot.ext4" # <-- For this one
}
```

Afin de charger la bonne image dans la bonne partition sur la carte SD.

Maintenant, il faut automatiser la création de l'image boot.ext4, pour ce faire, il faut modifier le fichier `post-build.sh` et rajouter cette partie là à la fin du fichier :
```
# --- Create boot.ext4 ---

# 1. Copy boot files from the binaries directory to a temporary location
cp "${BINARIES_DIR}/boot.scr" "${BOOT_TMP_DIR}/"
cp "${BINARIES_DIR}/kernel_fdt.itb" "${BOOT_TMP_DIR}/"

# 2. Create a 64MB ext4 image named boot.ext4 from the temporary directory
echo "Creating boot.ext4 with boot files..."
mkfs.ext4 -d "${BOOT_TMP_DIR}" -L boot "${BINARIES_DIR}/boot.ext4" 64M

# 3. Clean up the temporary directory
rm -rf "${BOOT_TMP_DIR}"

echo "Successfully created boot.ext4"
```

Après ça, on peut relancer un build complet.

## Question 5

### A

Pour cette partie, on peut regarder dans le dossier `./output/host/bin/` pour trouver toute la toolchain de cross-compilation.

Avec la commande `ls ./output/host/bin | grep gcc`, on peut trouver plusieurs compiler. Celui qui nous interesse est l'executable `aarch64-linux-gcc`.

Une fois cela fait, nous pouvons générer un petit fichier testant un stack smash, dans notre cas [stack_smasher.c](./stack_smasher.c).

Pour le cross-compiler, nous pouvons utiliser la commande `./output/host/bin/aarch64-linux-gcc <nom du fichier>`, dans notre cas : `./output/host/bin/aarch64-linux-gcc stack_smasher.c`.

Ensuite, on peut vérifier que le fichier a été correctement compilé avec comme target aarch64 avec la commande `file <fichier compilé>` pour nous permettre de voir l'architecture cible

Pour compiler le fichier avec les protections anti-smash, nous pouvons rajouter le flag `-fstack-protector-all` à la commande, ce qui nous donnes :

```
./output/host/bin/aarch64-linux-gcc -fstack-protector-all -o stack.elf stack_smasher.c
```

Ensuite, pour dé-compiler le fichier, on peut utiliser l'executable `./output/host/bin/aarch64-linux-objdump` avec la commande :

```bash
./output/host/bin/aarch64-linux-objdump -d smash.elf
```

Ce qui va output le fichier décompilé, et on peut remarquer, par exemple, un branch and link vers le flag : `__stack_chk_fail`, ce qui nous indique que la protection a bien été mise en place

### C

Pour ce point, la première chose demandée était de contrôler la taille du fichier `u-boot.bin`, on peut réaliser ça avec la commande :

```bash
ls -alh output/build/uboot-2020.10-rc5/ | grep u-boot.bin
# Output : 597K
```

Ensuite, on peut cloner le repository u-boot, et checkout la bonne version avec :

```bash
git clone https://github.com/u-boot/u-boot.git && \
cd u-boot && \
git checkout -b ses v2020.10-rc5
```

Ensuite, on peut analyser le `Makefile` de u-boot, en cherchant pour la chaine : `-fno-stack-protector` qui indique au compilateur de retirer les protections de stacks, on voit une correspondance à la ligne 680.

On peut donc remplacer cette ligne par celle-ci :
```bash
KBUILD_CFLAGS += $(call cc-option,-fstack-protector)
```

Ensuite, il ne faut pas oublier de rajouter la méthode `__stack_chk_fail` et la variable globale `__stack_chk_guard`, sinon la compilation ne marchera pas. On peut donc rajouter le fichier `<u-boot git clone>/lib/stackprot.c` :
```c
// SPDX-License-Identifier: GPL-2.0+
/*
 *  Copyright 2021 Broadcom
 */

#include <common.h>
#include <asm/global_data.h>

DECLARE_GLOBAL_DATA_PTR;

unsigned long __stack_chk_guard = (unsigned long)(0xfeedf00ddeadbeef & ~0UL);

void __stack_chk_fail(void)
{
	void *ra;

	ra = __builtin_extract_return_addr(__builtin_return_address(0));
	panic("Stack smashing detected in function:\n%p relocated from %p",
	      ra, ra - gd->reloc_off);
}
```

On peut suite en créer un patch avec :
```bash
git add . && \
git commit -m "Created patch for stack smashing protection" && \
git format-patch -1 && \
```

Ce qui nous donnes un patch ressemblant à [celui ci](./002-uboot-makefile-smash-protection-patch), il faut ensuite le rajouter dans le dossier `./board/friendlyarm/nanopi-neo-plus2/patches/uboot`

On peut ensuite nettoyer le dossier `output/build/uboot-***` avec la commande `rm -r output/build/uboot-<version uboot>`

Ensuite, recompiler uboot avec `make uboot-rebuild`.

Vérifier que les patchs sont correctement appliqués dans les logs.

Ensuite, vérifier que la stack smash protection soit bien appliquée avec par exemple :

```bash
cd <buildroot folder>
cd output
./host/bin/aarch64-linux-objdump -d ./build/uboot-<version uboot>/drivers/mmc/mmc.o | grep -A2 -B2 "stack_chk"
# This command should have some output -> indicating that the functions/variables are used in this file ! So the check is working
```


La nouvelle taille du fichier `u-boot.bin` peut être inspectée avec la commande :
```bash
ls -alh output/build/uboot-2020.10-rc5/ | grep u-boot.bin
# Ouput : 621K ==> 4% increase from 597K
```


Une fois cette étape faite. On peut tout nettoyer (`make clean`), et relancer la compilation totale avec `make -j <n_jobs>`.

Puis flash la SD, et vérifier que le système boot correctement.

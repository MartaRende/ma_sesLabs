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
fatload mmc 0:1 0x40000000 kernel_fdt.itb
```

On peut ensuite inspecter l'image chargée en mémoire avec :
```bash
iminfo 0x40000000
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
bootm 0x40000000
```

Afin de boot l'image. Et comme prévu, on a l'erreur : `Image too large: increase CONFIG_SYS_BOOTM_LEN`.

Cette erreur nous indique que le buffer alloué par U-Boot en mémoire est trop petit pour notre FIT. Il faudrait donc rebuild U-Boot avec un `CONFIG_SYS_BOOTM_LEN` agrandi.

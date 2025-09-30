# Rapport de Laboratoire : Initial Setup – NanoPi Neo Plus2 avec Buildroot

## 1. Introduction
L'objectif de ce laboratoire est de préparer l'environnement de développement pour le NanoPi Neo Plus2, compiler Buildroot pour générer une image SD contenant Linux, et connecter le NanoPi à un ordinateur hôte pour obtenir un shell fonctionnel. Les étapes principales comprennent :
- Installation et configuration des outils de développement (Git, Docker, VSCode, balenaEtcher, communication série).
- Mise en place de l’environnement containerisé pour le développement.
- Téléchargement, configuration et compilation de Buildroot.
- Création et inspection de l'image SD.
- Connexion et démarrage du NanoPi.


## 2. Installation des outils

### 2.1 Git
Installation de git
- Linux : `sudo apt-get install git`

### 2.2 Docker

* `sudo apt-get install docker.io docker-compose-v2`


### 2.3 Visual Studio Code

* Installation via [code.visualstudio.com](https://code.visualstudio.com/) ou gestionnaire de paquets sur Linux.
* Installer l’extension **Dev Containers** pour gérer automatiquement les conteneurs Docker depuis VSCode.

### 2.4 balenaEtcher

* `dd` a été utilisé

### 2.5 Communication série

* Linux : `picocom`

* Installation : `sudo apt-get install picocom`

---

## 3. Préparation de l’environnement de développement

1. Installer l’extension **Dev Containers** dans VSCode.
2. Créer le groupe Docker (si ce n’est pas déjà fait) et ajouter à l'utilisateur :

    ```bash
    sudo groupadd docker  # crée le groupe s’il n’existe pas
    sudo usermod -aG docker $USER
    newgrp docker  # recharge le groupe sans déconnexion
    ```

    Déconnectez-vous/reconnectez-vous si nécessaire pour appliquer les changements.

3. Ouvrir le dossier du projet contenant un fichier `.devcontainer`.
4. Accepter la proposition de VSCode : “Reopen in Container”.
5. Attendre la construction et le lancement automatique du conteneur Docker.
6. Développer dans VSCode à l’intérieur du conteneur avec toutes les dépendances prêtes.

1. Cloner le dépôt des ressources :

```bash
git clone https://github.com/MA-SeS/resources
```

2. Ouvrir le dossier `labs` avec VSCode.
3. Recharger le dossier dans un **Dev Container** pour utiliser l’environnement containerisé.
4. Le workspace `/workspace` dans le conteneur est mappé au dossier `labs` sur la machine hôte.

---

## 4. Buildroot

### 4.1 Installation

```bash
git clone https://gitlab.com/buildroot.org/buildroot.git
cd buildroot
git checkout -b ses 2022.08.3
```

### 4.2 Configuration

* Copier les fichiers spécifiques au NanoPi :

```bash
cp -r ../resources/buildroot/board/* board/
cp -r ../resources/buildroot/configs/ses_defconfig configs/
```

* Charger la configuration :

```bash
make ses_defconfig
```

* Vérifier avec :

```bash
make menuconfig
```

You can check in : Build options ---> Location to save buildroot config = ./configs/ses_defconfig
You can check in : Kernel ---> Kernel version = 6.3.6

### 4.3 Compilation

* Compilation multi-threads :

```bash
make -j4
```

Nombre non défini de threads:

```bash
make -j
```

* Répertoires principaux :

  * `dl` : tarballs des packages téléchargés
  * `output/host/bin` : outils pour l’hôte (cross-compilation)
  * `output/build` : source et compilation des packages
  * `output/images` : SPL, U-Boot, DTB, kernel, rootfs, sdcard.img

---

## 5. Création de l’image rootfs

* Buildroot utilise `system/skeleton` pour le rootfs.
* Exemple de création d’une image ext4 :

```bash
dd if=/dev/zero of=image.ext4 bs=1024 count=65536
mkfs.ext4 -L myfs image.ext4
mount -o loop image.ext4 /mnt
cp -r files /mnt
umount /mnt
```

---

## 6. SD Card Image

* Post-build : `post-build.sh` génère `boot.scr` avec `mkimage`
* `genimage.sh` génère `sdcard.img` à partir de `genimages.cfg`
* Contenu typique de `sdcard.img` :

  * Partition 1 : FAT32, 64 MB, contient `Image` (kernel), `boot.scr`, DTB
  * Partition 2 : ext4, 200 MB, contient rootfs

### 6.1 Commandes utiles

* Inspecter les partitions :

```bash
fdisk -l sdcard.img
```

* Monter partitions :

```bash
sudo mount -r -o loop,offset=<OFFSET>,sizelimit=<SIZE> sdcard.img /tmp/part1
sudo mount -r -o loop,offset=<OFFSET>,sizelimit=<SIZE> sdcard.img /tmp/part2
```

* Unmount : `sudo umount /tmp/{part1,part2}`

---

## 7. U-Boot ITB

* `u-boot.itb` est un **FIT image** (Flattened Image Tree)
* Contient :

  * `u-boot.bin` (bootloader)
  * `bl31.bin` (ARM Trusted Firmware)
  * `sun50i-h5-nanopi-neo-plus2.dtb` (Device Tree)

---

## 8. Flash SD Card

```bash
sudo dd if=/path/to/output/images/sdcard.img of=/dev/sda bs=1M
sudo sync
```

---

## 9. Connexion et démarrage du NanoPi

1. Connecter :

   * USB network adapter → PC
   * Serial cable → PC
   * microSD → NanoPi
   * microUSB → alimentation
2. Utiliser `picocom` :

```bash
sudo picocom -b 115200 /dev/ttyUSB0
```



5. Login : `root` sans mot de passe


Et on est dans la machine !

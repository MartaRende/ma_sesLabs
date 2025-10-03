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
        U-Boot configuration fragments
```

Il faut ici saisir le chemin d'accès à notre fichier fragment, à savoir :

```bash
board/friendlyarm/nanopi-neo-plus2/uboot-extras.config
```

Sauver les modifiocations faites dans le menuconfig: 

Pour nettoyer l'image qu'on a build pour lab_1 fauire : 
```bash
make clean
```
Ensuite :

```bash
make 
```

Enfin il faut flash l'image sur léa SD donc : 

```bash
sudo dd if= /output/images/sdcard.img of=/dev/sda bs=1M
sudo sync
```

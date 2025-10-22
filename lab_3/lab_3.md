# Lab 3

## Question 1

Pour cette question, la première étape est de créer un code en C permettant d'inspecter certaines adresses mémoire.

Pour ce faire, le fichier [aslr_check.c](./aslr_check.c) a été créé

Ensuite, pour vérifier si la chaine de cross compilation génère un exécutable PIE, on peut compiler le fichier en utilisant :

```bash
./output/host/bin/aarch64-linux-gcc -o aslr_check ./aslr_check.c
# Should compile the file into aslr_check
readelf -h aslr_check | grep Type:
# Outputs Type: DYN (Position-Independent Executable file)
```

Cela nous indique que l'exécutable est bien un PIE

On peut ensuite déplacer ce script dans l'overlay du filesystem. En controllant la valeur de l'option `System configuration ---> Root filesystem overlay directory`, on peut voir qu'elle vaut : `board/friendlyarm/nanopi-neo-plus2/rootfs_overlay`.

On a donc déjà un fs overlay présent, il suffit donc de déplacer le fichier avec :

```bash
mv aslr_check ./board/friendlyarm/nanopi-neo-plus2/rootfs_overlay/usr/bin
```

On peut ensuite re-créer l'image de la carte SD, puis la flash.

Ensuite, dans le système cible :

```bash
/usr/bin/aslr_check
# Address of main (text)        : 0xaaaaded4f848
# Address of foo  (text)        : 0xaaaaded4f840
# Address of stack_var (stack)  : 0xffffff85fef4
# Address of heap (malloc)      : 0xaaab08d4c2a0
# Address of printf (libc)      : 0xffff94593230
```


Il faut ensuite refaire les étapes pour compiler le programme et flasher la carte SD afin de vérifier si les adresses sont différentes


Et, à la deuxième exécution, on a :
```bash
/aslr_check
# Address of main (text)        : 0xaaaadf0bc848
# Address of foo  (text)        : 0xaaaadf0bc840
# Address of stack_var (stack)  : 0xffffc9c9cb34
# Address of heap (malloc)      : 0xaaab022ac2a0
# Address of printf (libc)      : 0xffff9a460230
```


On peut voir que entre les deux exécutions, toutes les adresses ont changées. Ce qui nous indique que ASLR est activé pour toutes les sections.


Et, en lançant la commande :
```bash
cat /proc/sys/kernel/randomize_va_space
#Output : 2
```

Cela confirme nous observation.


## Question 2

Pour trouver la taille de notre kernel, on peut utiliser la commande :

```bash
ls -alh ./output/images | grep Image
# output :
# -rw-r--r-- 1 root root 38M Oct 13 16:09 Image
# So => 38M
```


Ensuite, pour compiler le kernel avec les optimisations pour la taille activée, il faut faire :

```bash
make linux-menuconfig
# General Setup ---> Compiler Optimization level ---> Optimize for size
```

Ensuite, on peut recompiler le noyau linux uniquement avec :

```bash
make linux-rebuild
```

Après que ce soit compilé, on peut contrôler la taille du noyau linux et voir la différence avec :

```bash
ls -alh ./output/images | grep Image
# output :
# -rw-r--r-- 1 root root  35M Oct 20 15:28 Image
# # So => 35M ==> -3M
```


Ensuite, on peut créer l'image et flash la carte SD. Puis, dans la cible :
```bash
zcat /proc/config.gz | grep CONFIG_CC_OPTIMIZE
# Output : CONFIG_CC_OPTIMIZE_FOR_SIZE=y
```

Ce qui nous valide bien que l'optimisation de taille est appliquée.

## Question 3

Pour sécuriser notre noyau linux, on peut modifier certaines options dans la configurations du noyau, en tapant la commande :

```bash
make linux-menuconfig
```

Voici une liste d'option à modifier :

```bash
# To remove the /proc/config.gz, which contains sensible informations about the kernel
General setup ---> < > Kernel .config support

# To remove raw access to the physical memory -> might break some features --> We will keep this on for now
Device Drivers ---> Characters devices ---> [*] /dev/mem virtual device support
# OR
# To limit access to raw memory while keeping more functionnalities
Kernel hacking  ---> [*] Filter access to /dev/mem
                ---> [*] Filter I/O access to /dev/mem

# To enable stack smashing protection for the kernel
General architecture-dependent options  ---> [*] Stack Protector buffer overflow detection
                                        ---> [*] Strong Stack Protector

# This should be on by default -> randomize the memory adress of the kernel
Kernel Features ---> [*] Randomize the address of the kernel image

# Restrict access to kernel logs
Security options ---> [*] Restrict unprivileged access to the kernel syslog

# Empty the memory content on free and alloc by default
Security options ---> Kernel hardening options ---> Memory initialization ---> [*] Enable heap memory zeroing on allocation by default
                                                                          ---> [*] Enable heap memory zeroing on free by default
# Prevent memcopy of obviously wrong adress range
Security options ---> [*] Harden memory copies between kernel and userspace

# To protect basic functions against buffer overflows
Security options ---> [*] Harden common str/mem functions against buffer overflows
```

Ensuite, on peut compiler à nouveau avec :

```bash
make clean
make -j <n_jobs>
```

Ensuite, flasher la carte SD à nouveau, et vérifier que la cible boot toujours.

Pour tester certaines options en live, on peut par exemple tester la restriction de lecture de `/dev/mem` en faisant :

```bash
cat /dev/mem
# Error : bad adress
```


Cette solution n'est pas applicable à grande échelle, et il faut se tenir à jour/connaître par coeur toutes les options à activer. C'est pour cela que des projets comme [kernel-hardening-checker](https://github.com/a13xp0p0v/kernel-hardening-checker) existent.

On peut donc cloner ce repository et générer un config file avec :
```bash
git clone https://github.com/a13xp0p0v/kernel-hardening-checker.git && \
cd kernel-hardening-checker && \
./bin/kernel-hardening-checker -g ARM64 >> <path_to_buildroot>/board/friendlyarm/nanopi-neo-plus2/linux-extras.config
```

Cela va nous donner un fichier ressemblant à [linux-extras.config](./linux-extras.config)

Ensuite, on peut vérifier le bon fonctionnement en compilant la solution entière, flashant la carte SD et testant le bon fonctionnement de la cible.

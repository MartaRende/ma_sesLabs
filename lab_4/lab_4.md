# Lab 4

## Question 1

To download openssh, you can use this command :
```bash
curl https://ftp.openbsd.org/pub/OpenBSD/OpenSSH/portable/openssh-9.8.p1.tar.gz -o openssh-9.8.p1.tar.gz
```

And the PGP signature with :

```bash
curl https://ftp.openbsd.org/pub/OpenBSD/OpenSSH/portable/openssh-9.8p1.tar.gz.asc -o openssh-9.8p1.tar.gz.asc
```


To check the signature, you can use :

```bash
gpg --verify openssh-9.8p1.tar.gz.asc
# gpg: assuming signed data in 'openssh-9.8p1.tar.gz'
# gpg: Signature made Mon Jul  1 06:37:17 2024 CEST
# gpg:                using RSA key 7168B983815A5EEF59A4ADFD2A3F414E736060BA
# gpg: Can't check signature: No public key
```

As we can see, we don't have the corresponding public key for this signature, so we can import it with :

```bash
gpg --keyserver hkps://keyserver.ubuntu.com --recv-keys 7168B983815A5EEF59A4ADFD2A3F414E736060BA
# gpg: key 2A3F414E736060BA: public key "Damien Miller <djm@mindrot.org>" imported
# gpg: Total number processed: 1
# gpg:               imported: 1
```


And now, the signature is valid :

```bash
gpg --verify openssh-9.8p1.tar.gz.asc
# gpg: assuming signed data in 'openssh-9.8p1.tar.gz'
# gpg: Signature made Mon Jul  1 06:37:17 2024 CEST
# gpg:                using RSA key 7168B983815A5EEF59A4ADFD2A3F414E736060BA
# gpg: Good signature from "Damien Miller <djm@mindrot.org>" [unknown]
# gpg: WARNING: This key is not certified with a trusted signature!
# gpg:          There is no indication that the signature belongs to the owner.
# Primary key fingerprint: 7168 B983 815A 5EEF 59A4  ADFD 2A3F 414E 7360 60BA
```

Then, we can extract the folder and move into it :

```bash
tar xzf openssh-9.8p1.tar.gz
cd openssh-9.8p1
```


Then, we can compare both compilation logs with :

```bash
./configure > ../config_with_hardening.log 2>&1
cd ..
rm -r openssh-9.8p1
tar xzf openssh-9.8p1.tar.gz
cd openssh-9.8p1
./configure --without-hardening > ../config_without_hardening.log 2>&1
diff -u ../config_with_hardening.log ../config_without_hardening.log | less
# -      Linker flags:  -Wl,-z,relro -Wl,-z,now -Wl,-z,noexecstack -fstack-protector-strong -pie
# +      Linker flags:  -fstack-protector-strong
```

We can see that in both cases, the fstack-protector-strong flag was there, but that there is multiple options that were not added. Those options are here to protect/mitigate different exploits by for example creating a position independent executable etc...

## Question 2

In order to change the `openssh` version, we can inspect the file `<buildroot_folder>/package/openssh/openssh.mk` with the command :

```bash
head -20 ./package/openssh/openssh.mk
# .......
# OPENSSH_VERSION_MAJOR = 9.1
# OPENSSH_VERSION_MINOR = p1
# .......
```

Those are the two values we want to change, we can change them to `9.8` and `p1`

If you launch the rebuild now with `make openssh-rebuild`, there should be an error telling you that there is no hash for this package. This is expected.

To add the new hash, first, compute it with :

```bash
sha256sum ./dl/openssh/openssh-9.8p1.tar.gz
# Output : dd8bd002a379b5d499dfb050dd1fa9af8029e80461f4bb6c523c49973f5a39f3  ./dl/openssh/openssh-9.8p1.tar.gz
```

Now take that hash, and add it to the file `./package/openssh/openssh.hash` :

```bash
sha256 dd8bd002a379b5d499dfb050dd1fa9af8029e80461f4bb6c523c49973f5a39f3 openssh-9.8p1.tar.gz
```

Now you can rebuild the package with :

```bash
make openssh-rebuild
```

Then, we can rebuild the sd card image, flash it, and boot on it from the target. Then we can run :

```bash
ssh -V
# Output : OpenSSH_9.8p1, OpenSSL 1.1.1q  5 Jul 2022
```

To check that the new version of OpenSSH was correctly installed !

## Question 3

Now, on the target, we can run :
```bash
ls /etc/ssh
# moduli                    ssh_host_ecdsa_key.pub    ssh_host_rsa_key
# ssh_config                ssh_host_ed25519_key      ssh_host_rsa_key.pub
# ssh_host_ecdsa_key        ssh_host_ed25519_key.pub  sshd_config
```

As we can see, we have mutliple keys that we need to remote, so we can run :

```bash
rm /etc/ssh/ssh_host*
```

To then generate keys, you can use :

```bash
ssh-keygen -t rsa -b 4096 -f /root/.ssh/ssh_host_rsa -N ""
ssh-keygen -t dsa -b 1024 -f /root/.ssh/ssh_host_dsa -N ""
ssh-keygen -t ecdsa -b 512 -f /root/.ssh/ssh_host_ecdsa -N ""
ssh-keygen -t ed25519 -b 256 -f /root/.ssh/ssh_host_ed25519 -N ""
```

To make this change persistent, we can add this to a file in the rootfs overlay.

You can take [this file](./S50sshd) and move it to `<board_folder>/rootfs_overlay/etc/init.d/S50sshd`


Then, to configure the options needed for the sshd service, you can take the source `/etc/ssh/sshd_config` file, and edit to to match [this one](./sshd_config). Or you can just copy it to `<board_folder>/rootfs_overlay/etc/ssh/sshd_config`.

You can then rebuild the entire image, and test the changes by connecting in ssh to the target, or scanning the port with nmap

# TPM

## Question 1

To create a primary key in owner hierarchy, we can use this command :
```bash
tpm2_createprimary -C o -g sha256 -G rsa2048 -c primary.ctx
```

And we have a response like this : 
```bash

name-alg:
  value: sha256
  raw: 0xb
attributes:
  value: fixedtpm|fixedparent|sensitivedataorigin|userwithauth|restricted|decrypt
  raw: 0x30072
type:
  value: rsa
  raw: 0x1
exponent: 65537
bits: 2048
scheme:
  value: null
  raw: 0x10
scheme-halg:
  value: (null)
  raw: 0x0
sym-alg:
  value: aes
  raw: 0x6
sym-mode:
  value: cfb
  raw: 0x43
sym-keybits: 128
rsa: 990c7f6c7d923ce8c851da9bee80fac0f74d80172ca1b920798f950bed93de969d8468b2fe98803cf80977123e8b8437b9391a5bbb428a9f7b1924f0d2a045e507dc85e9a2a06395e7622d0f84e11a2e2eb2bdee7be80b2a04173561042a94fb7afee41668b45a2ce4bbf433a93a49dab5b5b518319e8f5eb05e5e23ebac14572571fba16c2213c778b9920875e45180588ad284dc3c64376a2ebbde30b25d44c8107f8490497f0f13f9f85711784c710f3cffc31c5264f4532585da5f016f7318ce4e2e4510fff1f1bf26d540a64cb74b43832fbc5d61b7b0a221ee7ade935419f7d6b3c0a71f4f31a429418e770aa146d2fca033f6e78ec81a44ff60919979
```


Now, we need to check the transient and presistent areas :

```bash
tpm2_getcap handles-transient
- 0x80000000

tpm2_getcap handles-persistent
# This is empty for now
```

Now we can test to flush our transient area :

```bash
tpm2_flushcontext -t
```

And if we print the content of the transient area, the key has correctly disappeared :

```bash
tpm2_getcap handles-transient
# nothing prints out
```

Now, we need to save the primary key to NVRAM (persitent memory) :

```bash
tpm2_evictcontrol -C o -c primary.ctx 0x81000000
persistent-handle: 0x81000000
action: persisted
```

And now, we can see that a new handle appeared in the persistent storage :

```bash
tpm2_getcap handles-persistent  
- 0x81000000
```

## Question 2

To create a child key using the just created parent key, we can use this command :
```bash
tpm2_flushcontext -t
tpm2_create -C 0x81000000 -g sha256 -G rsa2048 -u child.pub -r child.priv
name-alg:
  value: sha256
  raw: 0xb
attributes:
  value: fixedtpm|fixedparent|sensitivedataorigin|userwithauth|decrypt|sign
  raw: 0x60072
type:
  value: rsa
  raw: 0x1
exponent: 65537
bits: 2048
scheme:
  value: null
  raw: 0x10
scheme-halg:
  value: (null)
  raw: 0x0
sym-alg:
  value: null
  raw: 0x10
sym-mode:
  value: (null)
  raw: 0x0
sym-keybits: 0
rsa: aa51cfbd8d01060ecb098dd80db9bfcd30dda33849e4bb6233b7c5171e098132a3aea3dab58c36d09a3237541959d8121ffb0cc04399e80cc832ceb6ab26127952538f02f57160216585c80693f3e47e3c9a275512ea9a0d9ca965c8dff4142809c8a4bbf5eebca3e0f4a801a2be8577e9e55a3f5c260befcf2fc53fc44f9290a74c52d4d169a45181869e50a63d78ea25d7ae7c0c401dd29a4603109a081afb0fd64d7c17fd5b6341ace086c27a4eb9ed21e5095cbf03f638a84f80996a98e14d5bb8185a5b71b07e03eab0ed370a90055fa76feaa0802e860d4f269fd86d9df6af69bc4bf88ceee53b8540d1ac942680d4a89e3ad0bfa131208383b7aa2b79
```

Now, we can load it in transient memory, and inspect the transient and persistent areas : 

```bash
tpm2_load -C 0x81000000 -u child.pub -r child.priv -c child.ctx
name: 000b08a9d9e10c5124aabc9ed87812f812727586837ece8432a7ca2e23a188d06578
tpm2_getcap handles-transient
- 0x80000001
tpm2_getcap handles-persistent
- 0x81000000
```

Now, if we clear the transient space, and then inspect it, we can see that the key has correctly disappeared :

```bash
tpm2_flushcontext -t
tpm2_getcap handles-transient 
# Nothing prints out
tpm2_getcap handles-persistent
- 0x81000000
# The primary key is still in NVRAM, as expected
```

Now, to save the child key in NVRAM, we need to re-load it, and save it using :

```bash
tpm2_load -C 0x81000000 -u child.pub -r child.priv -c child.ctx
name: 000b08a9d9e10c5124aabc9ed87812f812727586837ece8432a7ca2e23a188d06578
tpm2_evictcontrol -C o -c child.ctx 0x81000001
persistent-handle: 0x81000001
action: persisted
```

We can then flush, and check the areas to see if all is correct :

```bash
tpm2_flushcontext -t
tpm2_getcap handles-transient
# This returns nothing
tpm2_getcap handles-persistent
- 0x81000000 <- primary key
- 0x81000001 <- child key
```

## Question 3

First, we can create a dummy kernel file, and then extends the TPM PCR with our new value :
```bash
echo "Original Linux Kernel v1" > vmlinuz
tpm2_pcrextend 4:sha256=$(sha256sum vmlinuz | awk '{print $1}')
# We can now check the state of the PCR
tpm2_pcrread sha256:4
sha256:
  4 : 0x4AAA8BB5B0E265713E90C1AC73A88E23AAB564E9BCD9F244507D36594C1CE3F1
```

Now, we can create a policy digest that ensure that access is only granted if PCR 4 matches its current value : 
```bash
tpm2_createpolicy --policy-pcr -l sha256:4 -L kernel_policy.dat
```


Now, we can emulate the kernel update by creating a new (different) file, add its hash to the PCR, and update the policy : 
```bash
echo "New Linux Kernel v2" > vmlinuz.new
tpm2_pcrextend 4:sha256=$(sha256sum vmlinuz.new | awk '{print $1}')
tpm2_createpolicy --policy-pcr -l sha256:4 -L kernel_policy_new.dat
9b9329a00129ad735cb978b36d27b2a2e51e78eb42147d9194ae246f0a395b5a
```

## Question 4

The first (and most critical error) is this line `but passwords should remain unchanged for backward compatibility.` in the `SM-6` section, which tells us that the default password should be kept for backward compatibility.
This is a critical flaw, as once a manual as leaked, the password is officially public knowledge. This means that once an attacker gains access to a machine, he can use the same password to attack evvery machines.


A second flaw is located in the section `SM-3 d.3)`, which says that setting/changing/deleting security option/capability should be possible without an administrator approval. This is quite bad as it prevents auditing and creates a "shadow" configuration environment where changes can be made without being tracked or approved. It breaks the least privilege principle, as it allows unauthorized changes to be made to the system without proper authorization or auditing.

Those are the only 2 major flaws that we found. While there is some phrasing that could be improved to be more precise (and avoid possible confusion), these are the most significant issues.

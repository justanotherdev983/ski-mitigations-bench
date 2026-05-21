# Create custom/self-compiled binaries

If you don't want to use the provided prebuilt binaries inside bin/
you have the following options to run this:

##  1

### Create all binaries by yourself

This has a few consequences:
- You need to cross-compile the linux kernel locally
- You need to use the patches provided upstream for hp-sim on said kernel
- You need to compile ```bski``` and the boot-loader ```ski-bootloader``` that bski provides

This could take a while depending on your hardware, while the bski package(s) is rather small,
the linux kernel build could take a while.

## 2

### Build only bski

This will save a lot of time and will probably be enough for most people.
Do this if you want to (but not limited to:

- Use this benchmark/framework on non-available architecture binary
- Compile own program to mitigate supply-chain issue's
- You have some weird dynamically linked situation

## 3

Don't. Use the provided binaries in bin/

-----------------

At the end of the day it is your choice, but choose wisely :)


## How to use

There are 3 supplied scripts here:
- build_vmlinux.sh
- build_bski.sh
- build_all.sh

You will need to patch it according to the target cpu architecture for ```bski```. 

## Example usage
(note, for these examples you are inside the usr-custom-bin/ dir)

### 1

```bash
$ chmod +x build*.sh
$ ./build_all.sh --arch=aarch64

```

### 2

```bash
$ chmod +x build_bski.sh
$ ./build_bski.sh --arch=x86

```
### 3

```bash
$ chmod +x build_bski.sh
$ ./build_bski.sh --arch=riscv64

```

### 4

```bash
$ chmod +x build_bski.sh
$ ./build_bski.sh --host=x86_64 --cross-arch=riscv64

```

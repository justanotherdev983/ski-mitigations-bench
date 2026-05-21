# ski-mitigations-bench

This repo provides the resources and code to test the effects of adding
`mitigations=off` to the `bski` Ski IA64-simulator; specifically the `dhrystone` benchmark
provided by the rootfs by the epic-linux project: https://ftp.machine-hall.org/pub/epic-linux/filesystems/

In later versions this will be extended to a full minimal benchmark framework for `bski` and IA64 benchmarks.

# Status
WIP under construction, x86_64 tested and (sort of) working

# How to setup the environment

Make sure you download the 8192mib-ext4-es-current.img.xz image from epic-linux

```bash

$ git clone https://github.com/justanotherdev983/ski-mitigations-bench .

$ cp </PATH/TO/DOWNLOADED/ROOTFS/>8192mib-ext4-es-current.img.xz . # Or automatically from setup_env.sh

$ xz -d 8192mib-ext4-es-current.img.xz                            # Or automatically from setup_env.sh

$ chmod +x setup_env.sh

# ./setup_env

$ make

$ ./ski-mitigations-bench # or ./ski-mitigations-bench <N>
                    # for N number of runs for both control and mitigations=off

```

The full N=1000 benchmark will take a while (estimated ~24,5 hours on 5950X setup,
~65 hours on my dual xeon E5 2630 V3),
so during any part of the benchmark running, you can ctrl-c the program and resume any other time.


```bash
$ ./ski-mitigations-bench --resume
```

the program will auto-detect where it left off and the overall simulation time will only be the extra `bski`
startup and the last run will have to be re-done.

# Selecting (custom) root dir 

The base case is this:

```bash
$ ./ski-mitigations-bench --root .
```

This will take the cloned repo dir and create env/ folder.

You can specify any other custom dir (like /tmp for tmpfs, or any other dir)

```bash
$ ./ski-mitigations-bench --root <dir>
```

# Example usage

## 1

Starting the benchmark:

```bash
$ ./ski-mitigations-bench -n 10 --root . 
```
Then after cancelling the benchmark, resuming:

```bash
$ ./ski-mitigations-bench -n 10 --root . --resume
```

## 2

```bash
$ ./ski-mitigations-bench --root .
```

## 3

```bash
$ mkdir /tmp/ski-bench
$ ./ski-mitigations-bench -n 50 --root /tmp/ski-bench
```

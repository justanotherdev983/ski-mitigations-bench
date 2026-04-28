# ski-mitigations-bench

This repo provides the resources and code to test the effects of adding
`mitigations=off` to the `bski` Ski IA64-simulator; specifically the `dhrystone` benchmark
provided by the rootfs by the epic-linux project: https://ftp.machine-hall.org/pub/epic-linux/filesystems/

In later versions this will be extended to a full minimal benchmark framework for `bski` and IA64 benchmarks.

# Status
Not yet implemented/Under construction.

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

The full N=1000 benchmark will take a while (24,5 hours on 5950X setup),
so during any part of the benchmark running, you can ctrl-c the program and resume any other time.


```bash
$ ./ski-mitigations-bench --resume
```

the program will auto-detect where it left of and the overall simulation time will only be the extra `bski`
startup and the last run will have to be re-done.

# Store tmp files in /tmp

There is another option for ski-mitigations-bench `--tmp`

This will create an artifact folder in /tmp and all the runtime artifact will be stored in tmpfs,
this is only recommended for small N, as at /tmp will be reset at reboot/shutdown


```bash
$ ./ski-mitigations-bench --tmp
```





# Select own root_dir

This option for ski-mitigations-bench `--root <YOUR_ROOT_DIR>`

This will change the benchmark runner to use that root path instead of the default "/home/test/test",
as of writing.


```bash
$ ./ski-mitigations-bench --root <YOUR_ROOT_DIR>
```

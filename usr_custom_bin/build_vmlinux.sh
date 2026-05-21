#!/bin/bash

# WIP: DO NOT USE (YET)

## TODO: parse (--host=<arch> --cross-arch=<arch>) etc

mkdir -p scratch
git clone https://github.com/johnny-mnemonic/linux-ia64 --depth=1 scratch
git clone https://github.com/linux-ia64/ski -b hp-sim-for-linux scratch

# ...

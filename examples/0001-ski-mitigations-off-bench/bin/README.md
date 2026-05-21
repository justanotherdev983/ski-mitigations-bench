# Prebuilt binaries

This subsection provides prebuilt binaries for this ski mitigations=off benchmark

## Why?

Because we can run the benchmark on different arch'es, such as, but not limited to, RISC-V64, AArch64,
X86_64, X86, IA64, etc.

## Why not the ski-bootloader and the vmlinux image in arch file?

These are ran inside the bski simulator, these are IA64 binaries run on the simulator and are independent
of the host's cpu architecture.

## "I don't want to run arbitrary binaries from a stranger on the internet"

Good that you mention it, WIP for custom images and build steps to replicate what I have here coming soon^tm...

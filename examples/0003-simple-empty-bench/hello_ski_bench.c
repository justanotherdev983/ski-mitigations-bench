#define SKI_BENCH_IMPLEMENTATION
#include "ski-bench.h"

int main(int argc, char **argv) {

	(void)argc;
	(void)argv;

	ski_bench_ctx ctx = {0};
	char *bench_runs_arg = "";

	ctx.conf_a = (ski_bench_bski_config){
                .path = "bin/bski", // i dont like this
                .argv = {
                        "bski", // i dont like this
                        "-noconsole",
                        SKI_BENCH_PATH_SKI_BOOTLOADER,
                        SKI_BENCH_PATH_VMLINUX,
                        "root=/dev/sda",
                        "simscsi=./sd",
                        "init=/init",
                        "PATH=/bin:/sbin:/usr/bin:/usr/sbin", "rw",
                        "nomca",
                        bench_runs_arg,
                        NULL,
                },
        };

	ctx.conf_b = ctx.conf_a;
        ski_bench_bski_append_argv(&ctx.conf_b, "");

	printf("Hello ski bench!\n");
}

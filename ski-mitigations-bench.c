#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <math.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdint.h>
#include <gsl/gsl_statistics.h>
#include <gsl/gsl_cdf.h>
#include <stdbool.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>
#include <linux/limits.h>
#include <fcntl.h>
#include <dirent.h>
#include <errno.h>

// TODO: not make this hardcoded
#define PATH_ROOT 		"/home/test/test"
#define PATH_BSKI 		"/home/test/ski/src/bski"
#define PATH_SKI_BOOTLOADER 	"/home/test/ski/ski-bootloader/ski-bootloader"
#define PATH_VMLINUX		"/home/test/linux-ia64/vmlinux"
#define SKI_BOOT_ARGS		"PATH_SKI_BOOTLOADER PATH_VMLINUX root=/dev/sda simscsi=./sd init=/init" \
							"PATH=/bin:/sbin:/usr/bin:/usr/sbin rw nomca N_RUNS"
#define BSKI_ARGV_MAX_SIZE 32

typedef struct {
	double mean;
	double variance;
	double stand_deviation;
	double stand_error;
	size_t n;
} computed_stats;

typedef struct {
	char *path;
	char *argv[BSKI_ARGV_MAX_SIZE];
} bski_config;

typedef struct {
	size_t idx;
	char *option;
} argv_ctx;

bool argv_contains(argv_ctx *ctx, char **argv, char *str) {

	for (size_t i = 1; argv[i] != NULL; i++) {
		if (strcmp(argv[i], str) == 0) {
			ctx->idx = i;
			return true;
		}
	}

	return false;

}

char *argv_get_next_str(argv_ctx *ctx, char **argv) {

	return argv[++ctx->idx];

}

char *get_latest_outdir(const char *root_dir) {
	DIR *dir;
	struct dirent *entry;
	static char latest[PATH_MAX];
	char best[NAME_MAX] = {0};

	dir = opendir(root_dir);
	if (!dir) {
		perror("opendir");
		return NULL;
	}

	while ((entry = readdir(dir)) != NULL) {
		if (strncmp(entry->d_name, "ski-bench-output-", 17) != 0)
			continue;
		if (strcmp(entry->d_name, best) > 0)
			snprintf(best, sizeof(best), "%s", entry->d_name);
	}

	closedir(dir);

	if (best[0] == '\0')
		return NULL;

	snprintf(latest, sizeof(latest), "%s/%s", root_dir, best);

	return latest;
}

void print_welcome(char* n_runs_str, uint64_t n_runs) {
	size_t estimated_total_secs;
	size_t estimated_hours;
	size_t estimated_rem_mins;
	size_t estimated_rem_secs;


	printf("Welcome the the Ski mitigations=off benchmark\n");
	printf("You have selected N=%s\n", n_runs_str);

	estimated_total_secs = 2 * (n_runs * 44 + 4);

	estimated_hours = estimated_total_secs / 3600;
	estimated_rem_mins = (estimated_total_secs % 3600) / 60;
	estimated_rem_secs = estimated_total_secs % 60;

	printf("Estimated time: %zu hours, %zu mins, %zu secs\n",
			estimated_hours, estimated_rem_mins, estimated_rem_secs);


}

void pretty_print_stats(const computed_stats *base_stats, const computed_stats *mitoff_stats,
		const char *out, uint64_t n_runs, double p_val) {


	FILE *fd;

	fd = fopen(out, "w");
	if (!fd) {
		printf("[OOPS] fopen failed");
		return;
	}

	fprintf(fd, "-------------------------------------\n");
	fprintf(fd, "----------------SKI-BENCH--------------\n");
	fprintf(fd, "--------------RESULTS N=%ld---------------\n", n_runs);
	fprintf(fd, "-------------------------------------\n");


	fprintf(fd, "-------------------------------------\n");
	fprintf(fd, "BASE STATS:--------------------------\n");
	fprintf(fd, "mean: %f-----------------------------\n", base_stats->mean);
	fprintf(fd, "variance: %f-------------------------\n", base_stats->variance);
	fprintf(fd, "standard deviation: %f---------------\n", base_stats->stand_deviation);
	fprintf(fd, "standard error: %f-------------------\n", base_stats->stand_error);
	fprintf(fd, "-------------------------------------\n");

	fprintf(fd, "-------------------------------------\n");
	fprintf(fd, "MIT_OFF STATS:--------------------------\n");
	fprintf(fd, "mean: %f-----------------------------\n", mitoff_stats->mean);
	fprintf(fd, "variance: %f-------------------------\n", mitoff_stats->variance);
	fprintf(fd, "standard deviation: %f---------------\n", mitoff_stats->stand_deviation);
	fprintf(fd, "standard error: %f-------------------\n", mitoff_stats->stand_error);
	fprintf(fd, "-------------------------------------\n");

	fprintf(fd, "P value: %f--------------------------\n", p_val);

	// Small hack, cat the output to (broken now) stdout
	FILE *rd = fopen(out, "r");
	if (rd) {
		int c;
		while ((c = fgetc(rd)) != EOF)
			putchar(c);
		fclose(rd);
	}

	fclose(fd);

	return;
}

size_t fetch_dmips(const char *file_path, double *out, size_t max) {

	FILE *fd;
	char line[1024]; // TODO: this will overflow

	size_t n = 0;

	fd = fopen(file_path, "r");
	if (!fd) {
		printf("[OOPS] fopen failed");
		return 0;
	}

	while (fgets(line, sizeof(line), fd) && n < max) {
		char *ptr = strstr(line, "DMIPS:");
		if (ptr) {
			double dmips_val;
			int ret;

			ret = sscanf(ptr, "DMIPS: %lf", &dmips_val);
			if (ret != EILSEQ || ret != EINVAL || ret != ENOMEM)
				out[n++] = dmips_val;
			else
				printf("[DEV] sscan failed");

		}
	}

	fclose(fd);
	return n;
}

static size_t count_dmips(const char *path)
{
	FILE *fd;
	char line[1024];
	size_t n = 0;

	fd = fopen(path, "r");
	if (!fd)
		return 0;

	while (fgets(line, sizeof(line), fd)) {
		if (strstr(line, "DMIPS:"))
			n++;
	}

	fclose(fd);
	return n;
}


bench_phase fetch_phase(const char *outdir, size_t n_runs) {
	char path[PATH_MAX];

	snprintf(path, sizeof(path), "%s/base.log", outdir);
	if (count_dmips(path) < n_runs)
		return PHASE_BASE;

	snprintf(path, sizeof(path), "%s/mit_off.log", outdir);
	if (count_dmips(path) < n_runs)
		return PHASE_MITOFF;

	return PHASE_DONE;
}

void calc_joined_stats(bench_phase phase) {
	return;
}


double calc_welch_p_val(const computed_stats *base_stats, const computed_stats *mitoff_stats) {

	double stand_err_diff;
	double base_sqrd_stand_err;
	double mitoff_sqrd_stand_err;
	double num;
	double den;
	double t_val;
	double deg_free;


	if (base_stats->n < 2 || mitoff_stats->n < 2)
		return NAN;

	base_sqrd_stand_err   = base_stats->variance / base_stats->n;
	mitoff_sqrd_stand_err = mitoff_stats->variance / mitoff_stats->n;

	stand_err_diff = sqrt(base_sqrd_stand_err + mitoff_sqrd_stand_err);
	if (stand_err_diff == 0.0)
		return 1.0;

	t_val = fabs(base_stats->mean- mitoff_stats->mean) / stand_err_diff;

	num = pow(base_sqrd_stand_err + mitoff_sqrd_stand_err, 2);

	den = (base_sqrd_stand_err * base_sqrd_stand_err) / (base_stats->n - 1)
		+ (mitoff_sqrd_stand_err* mitoff_sqrd_stand_err) / (mitoff_stats->n - 1);

	deg_free = num / den;

	return 2.0 * gsl_cdf_tdist_Q(t_val, deg_free);

}


double calc_stats(computed_stats *base_stats, computed_stats *mitoff_stats, const char *outdir_path,
		uint64_t n_runs) {


	size_t n_fetched_base 	= 0;
	size_t n_fetched_mitoff = 0;

	double *base_dmips   = (double*)malloc(sizeof(double) * n_runs);
	double *mitoff_dmips = (double*)malloc(sizeof(double) * n_runs);

	if (!base_dmips || !mitoff_dmips) {
		printf("[DEV] malloc failed, this is bad"); // TODO: handle more gracefully
		return NAN;
	}

	char base_fetch_log_path[PATH_MAX];
	char mitoff_fetch_log_path[PATH_MAX];

	int base_print_res = snprintf(base_fetch_log_path, sizeof(base_fetch_log_path),
			"%s/base.log", outdir_path);

	if (base_print_res < 0)
		printf("[DEV] snprintf failed, this is bad"); // TODO: handle more gracefully


	int mitoff_print_res = snprintf(mitoff_fetch_log_path, sizeof(mitoff_fetch_log_path),
			"%s/mit_off.log", outdir_path);

	if (mitoff_print_res < 0)
		printf("[DEV] snprintf failed, this is bad"); // TODO: handle more gracefully

	n_fetched_base   = fetch_dmips(base_fetch_log_path, base_dmips, n_runs);
	n_fetched_mitoff = fetch_dmips(mitoff_fetch_log_path, mitoff_dmips, n_runs);

	base_stats->mean 		= gsl_stats_mean(base_dmips, 1, n_fetched_base);
	base_stats->variance	 	= gsl_stats_variance(base_dmips, 1, n_fetched_base);
	base_stats->stand_deviation 	= gsl_stats_sd(base_dmips, 1, n_fetched_base);
	base_stats->stand_error	 	= base_stats->stand_deviation/sqrt(n_fetched_base);
	base_stats->n			= n_fetched_base;

	mitoff_stats->mean 		= gsl_stats_mean(mitoff_dmips, 1, n_fetched_mitoff);
	mitoff_stats->variance	 	= gsl_stats_variance(mitoff_dmips, 1, n_fetched_mitoff);
	mitoff_stats->stand_deviation 	= gsl_stats_sd(mitoff_dmips, 1, n_fetched_mitoff);
	mitoff_stats->stand_error	= mitoff_stats->stand_deviation/sqrt(n_fetched_mitoff);
	mitoff_stats->n			= n_fetched_mitoff;

	double p_val 			= calc_welch_p_val(base_stats, mitoff_stats);

	free(base_dmips);
	free(mitoff_dmips);

	return p_val;
}

void bski_append_argv(bski_config *conf, char* arg) {

	int i = 0;

	while (conf->argv[i] != NULL) {
		i++;
	}

	conf->argv[i] = arg;
	conf->argv[i + 1] = NULL;
}

pid_t bski_runner(const bski_config* config, const char *out) {
	pid_t pid_bski;
	pid_t pid_writer;

	int pipefd[2];

	printf("[parent] fork+exec: %s", config->path);
	for (size_t i = 0; config->argv[i] != NULL; i++)
		printf(" %s", config->argv[i]);
	printf("\n");

	if (pipe(pipefd) < 0) {
		perror("pipe");
		return -1;
	}

	pid_bski = fork();
	if (pid_bski == 0) {

		close(pipefd[0]);
		dup2(pipefd[1], STDOUT_FILENO);
		dup2(pipefd[1], STDERR_FILENO);
		close(pipefd[1]);

		execvp(config->path, config->argv);
		perror("execvp");
		printf("WELP, this is akward, execve shouldnt return\n");
		printf("You really think im going to list you the execve error???\n");

		_exit(1);
	}

	close(pipefd[1]);

	pid_writer = fork();
	if (pid_writer == 0) {
		int fd;
		char buf[4096];
		ssize_t n;

		fd = open(out, O_WRONLY | O_CREAT | O_APPEND, 0644);
		if (fd < 0) {
			perror("open");
			_exit(1);
		}

		while ((n = read(pipefd[0], buf, sizeof(buf))) > 0)
			write(fd, buf, n);
		close(fd);
		close(pipefd[0]);
		_exit(0);
	}

	close(pipefd[0]);
	return pid_bski;
}

void bski_kill(pid_t pid, uint64_t n_runs) {
	// Due to bski not being able to quit on its own
	// we need to manually figure out when we kill
	// TODO: XXX binary search the time for each machine
	// and then run with that, this will create a little overhead
	// but can save alot of times with greater N
	sleep((n_runs * 44 + 4));
							//n_runs * 44 + 4;
	kill(pid, SIGKILL);
	waitpid(pid, NULL, 0);
}

void run_base(const bski_config *config, uint64_t n_runs, const char *outdir) {
	pid_t pid;
	char path[PATH_MAX];

	snprintf(path, sizeof(path), "%s/base.log", outdir);
	pid = bski_runner(config, path);

	bski_kill(pid, n_runs);
}

void run_mitigations_off(const bski_config *config, uint64_t n_runs, const char *outdir) {
	pid_t pid;
	char path[PATH_MAX];

	snprintf(path, sizeof(path), "%s/mit_off.log", outdir);
	pid = bski_runner(config, path);

	bski_kill(pid, n_runs);
}

int main(int argc, char** argv) {
	char* n_runs_str;
	uint64_t n_runs;

	char bench_runs_arg[64];

	computed_stats base_stats;
	computed_stats mitoff_stats;

	if (argc < 2)
		n_runs_str = "1000";
	else
		n_runs_str = argv[1];

	n_runs = strtol(n_runs_str, NULL, 0);


	snprintf(bench_runs_arg, sizeof(bench_runs_arg), "BENCH_RUNS=%s", n_runs_str);

	bski_config base_config = {
		.path = PATH_BSKI,
		.argv = {
			PATH_BSKI,
			"-noconsole",
			PATH_SKI_BOOTLOADER,
			PATH_VMLINUX,
			"root=/dev/sda",
			"simscsi=./sd",
			"init=/init",
			"PATH=/bin:/sbin:/usr/bin:/usr/sbin", "rw",
			"nomca",
			bench_runs_arg,
			NULL,
		},
	};

	bski_config mitoff_config = base_config;
	bski_append_argv(&mitoff_config, "mitigations=off");

	print_welcome(n_runs_str, n_runs);


	argv_ctx ctx;
	if (argv_contains(&ctx, argv, "--tmp"))
		chdir("/tmp");

	chdir(PATH_ROOT); // XXX: Hacky, see below
	char outdir[PATH_MAX];
        time_t now = time(NULL);
        strftime(outdir, sizeof(outdir), "ski-bench-output-%Y%m%d_%H%M%S", localtime(&now));
	mkdir(outdir, 0755);

	if (argv_contains(&ctx, argv, "--resume")) {
		// todo, search for the newest outdir and get the latest N
		// and continue from last gotten DMIPS after new ski boot and BENCH_RUNS - latest N
	}

	if (argv_contains(&ctx, argv, "--multi")) {
		// todo, get the num of jobs and use pthread so we compare multi and single also
	}

	if (argv_contains(&ctx, argv, "--root")) {
		char *user_root_path = argv_get_next_str(&ctx, argv); // TODO: sanitize user input, this is bad
								      // TODO: impl
	}

	run_base(&base_config, n_runs, outdir);
	run_mitigations_off(&mitoff_config, n_runs, outdir);

	double p_val = calc_stats(&base_stats, &mitoff_stats, outdir, n_runs);

	char print_results_log_path[PATH_MAX];
	snprintf(print_results_log_path, sizeof(print_results_log_path),
			"%s/results.log", outdir);


	pretty_print_stats(&base_stats, &mitoff_stats, print_results_log_path, n_runs, p_val);

	return 0;

}

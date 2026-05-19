CC = gcc
CFLAGS = -O2 -Wall -Wextra

all:
	$(CC) $(CFLAGS) ski-mitigations-bench.c -o ski-mitigations-bench -lm -lgsl -lgslcblas



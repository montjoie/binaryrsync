#define _LARGEFILE64_SOURCE

#include <stdio.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>

#define BUF_SIZE 4096

void usage(const char *argv[]) {
	printf("%s -h\n", argv[0]);
	printf("%s [--size x] [--pattern x] filename\n", argv[0]);
	printf("Default size is 1, default pattern is 0\n");
	printf("Size is the niumber of block of 4096o\n");
}

int main(const int argc, const char *argv[]) {
	int fd;
	int ret;
	int argi = 1;
	const char *dst = NULL;
	unsigned char *d;
	int foundone;
	unsigned int total = 1;
	unsigned char pattern = 0;

	if (argc <= 1) {
		usage(argv);
		return EXIT_SUCCESS;
	}

	if (strcmp(argv[argi], "-h") == 0) {
		usage(argv);
		return EXIT_SUCCESS;
	}
arg_start:
	foundone = 0;
	if (argi >= argc) {
		usage(argv);
		return EXIT_SUCCESS;
	}
	if (strcmp(argv[argi], "--size") == 0) {
		argi++;
		total = strtoul(argv[argi], NULL, 10);
		argi++;
		foundone = 1;
	}
	if (strcmp(argv[argi], "--pattern") == 0) {
		argi++;
		pattern = strtoul(argv[argi], NULL, 10);
		argi++;
		foundone = 1;
	}
	if (foundone == 1)
		goto arg_start;

	if (argc - argi < 1) {
		printf("Need more arguments\n");
		usage(argv);
		return EXIT_FAILURE;
	}
	if (argc - argi > 2) {
		printf("Extra arguments after source/dest\n");
		usage(argv);
		return EXIT_FAILURE;
	}

	dst = argv[argi];
	printf("TESTGEN %d x %d %x to %s\n", total, BUF_SIZE, pattern, dst);
	fd = open(dst, O_WRONLY| O_TRUNC| O_CREAT, S_IRWXU);
	if (fd < 0) {
		printf("FAILED to open %s %s\n", dst, strerror(errno));
		return -1;
	}

	d = malloc(BUF_SIZE);
	if (!d) {
		goto error;
	}
	memset(d, pattern, BUF_SIZE);

	while (total-- > 0) {
		printf("WRITE %d\n", total);
		ret = write(fd, d, BUF_SIZE);
		if (ret < 0) {
			printf("WRITE ERROR %d %s\n", ret, strerror(errno));
		}
	}

	free(d);
error:
	close(fd);
	return EXIT_SUCCESS;
}

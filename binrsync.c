#define _LARGEFILE64_SOURCE

#include <stdio.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>

#define BUF_SIZE 4096

/* FreeBSD does not have lseek64 */
#ifndef HAVE_LSEEK64
#define lseek64 lseek
#endif

#ifndef HAVE_OFF64_T
#define off64_t off_t
#endif

/* return 0 if same */
int compare(const unsigned char *s, unsigned const char *d, unsigned int tocomp) {
	unsigned int i;

	for (i = 0; i < tocomp; i++) {
		if (s[i] != d[i])
			return 1;
	}
	return 0;
}

void usage(const char *argv[]) {
	printf("%s -h\n", argv[0]);
	printf("%s [-s|-g] source destination\n", argv[0]);
}

int main(const int argc, const char *argv[]) {
	int fs, fd;
	unsigned char *s, *d;
	int ret, rets, retd;
	unsigned int pi;
	off64_t off, off_ret;
	unsigned long stat_sync = 0, stat_same = 0;
	float progress;
	off64_t dd_seek = 0;
	int tocomp;
	int doit = 0;
	int argi = 1;
	int debug = 0;
	const char *src = NULL;
	const char *dst = NULL;
	off64_t src_size;
	off64_t dst_size;
	int foundone;

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
	if (strcmp(argv[argi], "-g") == 0) {
		printf("Go mode\n");
		doit = 1;
		argi++;
		foundone = 1;
	}
	if (strcmp(argv[argi], "-s") == 0) {
		printf("Simulate mode\n");
		doit = 0;
		argi++;
		foundone = 1;
	}
	if (strcmp(argv[argi], "-d") == 0) {
		printf("Debug mode\n");
		debug = 1;
		argi++;
		foundone = 1;
	}
	if (strcmp(argv[argi], "--seek") == 0) {
		argi++;
		dd_seek = strtoul(argv[argi], NULL, 10);
		argi++;
		foundone = 1;
		printf("Seek %lu\n", dd_seek);
	}
	if (foundone == 1)
		goto arg_start;

	if (argc - argi < 2) {
		printf("Need more arguments\n");
		usage(argv);
		return EXIT_FAILURE;
	}
	if (argc - argi > 2) {
		printf("Extra arguments after source/dest\n");
		usage(argv);
		return EXIT_FAILURE;
	}

	/* verify source/dest exists*/
	src = argv[argi];
	if (access(src, F_OK) == 0) {
		printf("SOURCE %s exists\n", src);
	} else {
		printf("SOURCE %s does not exists exists\n", src);
		return -1;
	}
	argi++;
	dst = argv[argi];
	if (access(dst, F_OK) == 0) {
		printf("DEST %s exists\n", dst);
	} else {
		printf("DEST %s does not exists exists\n", dst);
		return -1;
	}

	printf("COPY FROM %s to %s\n", src, dst);
	fs = open(src, O_RDONLY);
	if (fs < 0) {
		printf("FAILED to open %s %s\n", src, strerror(errno));
		return -1;
	}
	fd = open(dst, O_RDWR);
	if (fd < 0) {
		printf("FAILED to open %s %s\n", dst, strerror(errno));
		close(fs);
		return -1;
	}

	src_size = lseek64(fs, 0L, SEEK_END);
	printf("SOURCE SIZE %lu\n", src_size);
	lseek64(fs, 0L, SEEK_SET);

	dst_size = lseek64(fd, 0L, SEEK_END);
	printf("DEST SIZE %lu\n", dst_size);
	lseek64(fd, 0L + dd_seek, SEEK_SET);

	if (dst_size < src_size) {
		printf("WARNING: destination is too small\n");
	}

	s = malloc(BUF_SIZE);
	if (!s) {
		goto error;
	}
	d = malloc(BUF_SIZE);
	if (!d) {
		goto error;
	}

	pi = 0;
retry:
	rets = read(fs, s, BUF_SIZE);
	if (rets < 0) {
		printf("ERROR: read source %d %s\n", rets, strerror(errno));
		goto error2;
	}
	if (rets == 0) {
		printf("FIN SOURCE\n");
		goto error2;
	}

	off = lseek64(fd, 0, SEEK_CUR);

	progress = (float)off / (float)src_size;
	if (debug == 0) {
		printf("\r progress %0.2f same=%lu sync=%lu", (100 * progress), stat_same, stat_sync);
		fflush(stdout);
	} else {
		printf("\tDEBUG: SEEK %ld (0x%lx) progress=%f\n", off, off, progress);
	}
	retd = read(fd, d, BUF_SIZE);
	if (retd < 0) {
		printf("ERROR: read dest %d %s\n", retd, strerror(errno));
		goto error2;
	}
	if (retd == 0) {
		printf("FIN DEST\n");
		goto error2;
	}

	if (rets < BUF_SIZE) {
		printf("partial source %d\n", rets);
	}
	if (retd < BUF_SIZE) {
		printf("partial dest %d\n", retd);
	}

	tocomp = BUF_SIZE;
	if (tocomp > rets)
		tocomp = rets;
	if (tocomp > retd)
		tocomp = retd;

	ret = compare(s, d, tocomp);
	if (ret == 1) {
		if (doit == 0) {
			/* no thing */
			stat_sync++;
		} else {
			off_ret = lseek64(fd, off, SEEK_SET);
			/* FIXME */
			if (off_ret == (off_ret - 1))
				printf("SEEK error %s\n", strerror(errno));
			ret = write(fd, s, rets);
			if (ret < 0) {
				printf("WRITE ERROR %d %s\n", ret, strerror(errno));
			}
			stat_sync++;
		}
		if (debug)
			printf("\tDEBUG: SYNC %u %d\n", pi, ret);
	} else {
		stat_same++;
		if (debug)
			printf("\tDEBUG: SAME %u\n", pi);
	}
	pi++;
	goto retry;

error2:
	printf("SAME %lu\n", stat_same);
	printf("SYNC %lu\n", stat_sync);
	free(d);
	free(s);
error:
	close(fd);
	close(fs);
	return EXIT_SUCCESS;
}

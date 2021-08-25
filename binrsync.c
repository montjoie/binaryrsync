#include <stdio.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>

#define BUF_SIZE 4096

/* return 0 if same */
int compare(const char *s, const char *d, unsigned int tocomp) {
	unsigned int i;

	for (i = 0; i < tocomp; i++) {
		if (s[i] != d[i])
			return 1;
	}
	return 0;
}

int main(const int argc, const char *argv[]) {
	int fs, fd;
	char *s, *d;
	int ret, rets, retd;
	unsigned int pi;
	off_t off, off_ret;
	unsigned int stat_sync = 0, stat_same = 0;
	int tocomp;
	int doit = 1;

	if (argc < 2) {
		return -1;
	}
	printf("COPY FROM %s to %s\n", argv[1], argv[2]);
	fs = open(argv[1], O_RDONLY);
	if (fs < 0) {
		printf("FAILED to open %s %s\n", argv[1], strerror(errno));
		return -1;
	}
	fd = open(argv[2], O_RDWR);
	if (fd < 0) {
		printf("FAILED to open %s %s\n", argv[2], strerror(errno));
		close(fs);
		return -1;
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

	off = lseek(fd, 0, SEEK_CUR);
	/*printf("SEEK %ld\n", off);*/
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
			off_ret = lseek(fd, off, SEEK_SET);
			if (off_ret == (off_ret - 1))
				printf("SEEK error %s\n", strerror(errno));
			ret = write(fd, s, rets);
			if (ret < 0) {
				printf("WRITE ERROR %d %s\n", ret, strerror(errno));
			}
			stat_sync++;
		}
		/*printf("SYNC %u %d\n", pi, ret);*/
	} else {
		stat_same++;
		/*printf("SAME %u\n", pi);*/
	}
	pi++;
	goto retry;

error2:
	printf("SAME %u\n", stat_same);
	printf("SYNC %u\n", stat_sync);
	free(d);
	free(s);
error:
	close(fd);
	close(fs);
	return 0;
}

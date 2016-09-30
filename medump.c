/**
 * MediaTek EEPROM dump utility
 *
 * Copyright (c) 2016, Sergey Ryazanov <ryazanov.s.a@gmail.com>
 */

#include <stdio.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <libgen.h>
#include <unistd.h>
#include <stdint.h>

#include <sys/types.h>
#include <sys/stat.h>

static uint8_t eep_buf[0x1000];		/* 4k buffer */
static unsigned eep_len;		/* Actual EERPOM size */

static int parse_file(const char *filename)
{
	int fd, ret;
	struct stat stat;

	fd = open(filename, O_RDONLY);
	if (-1 == fd) {
		fprintf(stderr, "Could not open input file %s: %s\n",
			filename, strerror(errno));
		return -1;
	}

	if (fstat(fd, &stat)) {
		fprintf(stderr, "Could not stat file %s: %s\n",
			filename, strerror(errno));
		close(fd);
		return -1;
	}

	if (!stat.st_size) {
		fprintf(stderr, "Input file is empty");
		close(fd);
		return -1;
	}

	if (stat.st_size > sizeof(eep_buf)) {
		fprintf(stderr, "Input file too big (%lu bytes), expect not more than %u bytes, file will be readed partially\n",
			stat.st_size, sizeof(eep_buf));
		eep_len = sizeof(eep_buf);
	} else if (stat.st_size % 2 != 0) {
		fprintf(stderr, "Input file size is not even (%lu bytes), will read one byte less\n",
			stat.st_size);
		eep_len = stat.st_size - 1;
	} else {
		eep_len = stat.st_size;
	}

	ret = read(fd, eep_buf, eep_len);
	if (ret != eep_len) {
		fprintf(stderr, "Could not read input file: %s\n",
			strerror(errno));
		close(fd);
		return -1;
	}

	close(fd);

	return 0;
}

static void usage(const char *name)
{
	printf(
		"MediaTek EEPROM dump utility\n"
		"Copyright (c) 2016, Sergey Ryazanov <ryazanov.s.a@gmail.com>\n"
		"\n"
		"Usage:\n"
		"  %s [-h] <eeprom.bin>\n"
		"\n"
		"Options:\n"
		"  -h       Print this help\n"
		"\n",
		name
	);
}

int main(int argc, char *argv[])
{
	const char *appname = basename(argv[0]);
	int opt, ret;

	if (argc <= 1)
		usage(appname);

	while ((opt = getopt(argc, argv, "h")) != -1) {
		switch (opt) {
		case 'h':
			usage(appname);
			return EXIT_SUCCESS;
		default:
			return EXIT_FAILURE;
		}
	}

	if (optind == argc) {
		fprintf(stderr, "Input file notspecified, use -h option to see more details\n");
		return EXIT_FAILURE;
	}

	ret = parse_file(argv[optind]);

	return ret ? EXIT_FAILURE : EXIT_SUCCESS;
}

/**
 * MediaTek EEPROM management utility
 *
 * Copyright (c) 2016-2020, Sergey Ryazanov <ryazanov.s.a@gmail.com>
 */

#include <stdio.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <libgen.h>
#include <unistd.h>
#include <stdint.h>
#include <endian.h>

#include <sys/types.h>
#include <sys/stat.h>

#include "mtkeepmgr.h"

extern struct chip_desc *__start___chips[];
extern struct chip_desc *__stop___chips;

#define for_each_chip(__chip, __i)					\
	for (__i = 0; __i < &__stop___chips - __start___chips; ++__i)	\
		if ((__chip = __start___chips[i]))	/* to skip possible padding */

static uint8_t eep_buf[0x1000];		/* 4k buffer */
static unsigned eep_len;		/* Actual EERPOM size */

uint16_t eep_read_word(const unsigned offset)
{
	uint16_t val;

	if (offset >= eep_len)
		return 0xffff;

	memcpy(&val, &eep_buf[offset], sizeof(val));

	return le16toh(val);
}

static int parse_file(const char *filename)
{
	int fd, ret;
	struct stat stat;
	uint16_t chipid, version;
	struct chip_desc *chip;
	int i;

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
		fprintf(stderr, "Input file too big (%lu bytes), expect not more than %zu bytes, file will be readed partially\n",
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

	printf("[EEPROM identification]\n");

	chipid = eep_read_word(E_CHIPID);
	printf("  ChipID        : %04Xh\n", chipid);
	version = eep_read_word(E_VERSION);
	printf("  Version       : %u.%u\n",
	       FIELD_GET(E_VERSION_VERSION, version),
	       FIELD_GET(E_VERSION_REVISION, version));

	for_each_chip(chip, i)
		if (chip->chipid == chipid)
			break;

	if (!chip || chip->chipid != chipid) {
		fprintf(stderr, "EEPROM dump is for unknown or unsupported chip\n");
		return -1;
	}

	printf("  Chip          : %s\n", chip->name);

	printf("\n");

	return chip->parse_func();
}

static void usage(const char *name)
{
	printf(
		"MediaTek EEPROM management utility\n"
		"Copyright (c) 2016-2020, Sergey Ryazanov <ryazanov.s.a@gmail.com>\n"
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
/**
 * MediaTek EEPROM management utility
 *
 * Copyright (c) 2016-2021, Sergey Ryazanov <ryazanov.s.a@gmail.com>
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

extern const struct connector_desc con_file;

/* The main utility execution context */
static struct main_ctx __mc;

uint16_t eep_read_word(struct main_ctx *mc, const unsigned offset)
{
	uint16_t val;

	if (offset >= mc->eep_len)
		return 0xffff;

	memcpy(&val, &mc->eep_buf[offset], sizeof(val));

	return le16toh(val);
}

static int parse_file(struct main_ctx *mc)
{
	uint16_t chipid, version;
	struct chip_desc *chip;
	int i;

	printf("[EEPROM identification]\n");

	chipid = eep_read_word(mc, E_CHIPID);
	printf("  ChipID        : %04Xh\n", chipid);
	version = eep_read_word(mc, E_VERSION);
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

	return chip->parse_func(mc);
}

#define CON_USAGE_FILE	"-F <eepdump>"

#define CON_OPTSTR	"F:"
#if 1
#define CON_USAGE	"{" CON_USAGE_FILE "}"
#else
#define CON_USAGE	CON_USAGE_FILE
#endif

static void usage(const char *name)
{
	printf(
		"MediaTek EEPROM management utility\n"
		"Copyright (c) 2016-2021, Sergey Ryazanov <ryazanov.s.a@gmail.com>\n"
		"\n"
		"Usage:\n"
		"  %s [-h] " CON_USAGE "\n"
		"\n"
		"Options:\n"
		"  -F <eepdump>\n"
		"           Read EEPROM dump from <eepdump> file.\n"
		"  -h       Print this help\n"
		"\n",
		name
	);
}

int main(int argc, char *argv[])
{
	const char *appname = basename(argv[0]);
	struct main_ctx *mc = &__mc;
	char *con_arg = NULL;
	int opt, ret;

	if (argc <= 1)
		usage(appname);

	while ((opt = getopt(argc, argv, CON_OPTSTR "h")) != -1) {
		switch (opt) {
		case 'F':
			mc->con = &con_file;
			con_arg = optarg;
			break;
		case 'h':
			usage(appname);
			return EXIT_SUCCESS;
		default:
			return EXIT_FAILURE;
		}
	}

	if (!mc->con) {
		fprintf(stderr, "Connector (data source) was not specified\n");
		goto exit;
	}

	mc->con_priv = malloc(mc->con->priv_sz);
	if (!mc->con_priv) {
		fprintf(stderr, "Unable to allocate memory for a connector private data\n");
		goto exit;
	}

	ret = mc->con->init(mc, con_arg);
	if (ret)
		goto exit;

	ret = parse_file(mc);

	mc->con->clean(mc);

exit:
	free(mc->con_priv);

	return ret ? EXIT_FAILURE : EXIT_SUCCESS;
}

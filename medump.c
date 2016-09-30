/**
 * MediaTek EEPROM dump utility
 *
 * Copyright (c) 2016, Sergey Ryazanov <ryazanov.s.a@gmail.com>
 */

#include <stdio.h>
#include <stdlib.h>
#include <libgen.h>
#include <unistd.h>

static int parse_file(const char *filename)
{
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

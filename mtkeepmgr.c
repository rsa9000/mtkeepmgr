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
extern const struct connector_desc con_usb;

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

static int act_eep_dump(struct main_ctx *mc, int argc, char *argv[])
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
		fprintf(stderr, "EEPROM dump is for unknown or unsupported chip (chipid:0x%04x)\n",
			chipid);
		return -1;
	}

	printf("  Chip          : %s\n", chip->name);

	printf("\n");

	return chip->parse_func(mc);
}

static const struct action {
	const char * const name;
	int (*func)(struct main_ctx *mc, int argc, char *argv[]);
} actions[] = {
	{
		.name = "dump",
		.func = act_eep_dump,
	}
};

#define CON_USAGE_FILE	"-F <eepdump>"
#ifdef CONFIG_CON_USB
#define CON_USAGE_USB	" | -U <dev-sel>"
#define CON_OPTSTR_USB	"U:"
#else
#define CON_USAGE_USB	""
#define CON_OPTSTR_USB	""
#endif

#define CON_OPTSTR	"F:" CON_OPTSTR_USB
#if defined(CONFIG_CON_USB)
#define CON_USAGE	"{" CON_USAGE_FILE CON_USAGE_USB "}"
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
		"  %s [-h] " CON_USAGE " [<action> [<actarg>]]\n"
		"\n"
		"Options:\n"
		"  -F <eepdump>\n"
		"           Read EEPROM dump from <eepdump> file.\n"
#ifdef CONFIG_CON_USB
		"  -U <dev-sel>\n"
		"           Work with USB device, which is specified by a selector <dev-sel>.\n"
		"           Utiltity supported a few types of selection rules.\n"
		"           To select device by a USB bus number and corresponding address on the\n"
		"           bus use '<busnum>:<devaddr>' format, where <busnum> and <devaddr>\n"
		"           should be specified in the decimal form (e.g. '1:193').\n"
		"           To select first device with a specific IDs, use <VID>:<PID>, where\n"
		"           <VID> and <PID> should be specified in the cannonical 4 symbol hex\n"
		"           form (e.g. '148f:7610'). This form could be used not only to narrow\n"
		"           device selection range, but also to force device selection, when\n"
		"           IDs of a new device is not yet embedded to the utility table of\n"
		"           supported devices.\n"
		"           To be absolutely specific, you could select a device by its path\n"
		"           (i.e. sequence of HUB ports from root to device) on a bus. Use this\n"
		"           format to utilize path selector:\n"
		"           '<busnum>/<hub1portnum>[/<hub2port>[/<hub3port>[...]]][/]'. Please\n"
		"           note optional leading path slash. If the leading slash is specified\n"
		"           then path treated as a required prefix of a full device path. If the\n"
		"           leading slash is omited, then path treated as an exact full device\n"
		"           path. So you could specify full device path or only a path to an USB\n"
		"           that you like to use to connect your device.\n"
		"           Several selectors could be specified at once by concatenating them\n"
		"           with comma (e.g. <busnum>:<devaddr>,<VID>:<PID>). Such format could\n"
		"           be used to force yet unknown device usage and in the same time select\n"
		"           one specific device of multiple connected to a host. NB: device\n"
		"           address and device path selectors are mutually exclusive and should\n"
		"           not be used together.\n"
		"           There is a special keyword 'any'. If it specified, then the utility\n"
		"           will open first device with a known VID/PID pair. This is useful\n"
		"           when you have only one device connected to the host and you do not\n"
		"           want to type a longer option argument.\n"
#endif
		"  -h       Print this help\n"
		"  <action> Optional argument, which specifies the <action> that should be\n"
		"           performed (see actions list below). If no action is specified, then\n"
		"           the 'dump' action is performed by default.\n"
		"  <actarg> Action argument if the action accepts any (see details below in the\n"
		"           detailed actions list).\n"
		"\n"
		"Available actions:\n"
		"  dump     Read & parse the EEPROM content and then dump parsed results to the\n"
		"           terminal (this is the default action).\n"
		"\n",
		name
	);
}

int main(int argc, char *argv[])
{
	const char *appname = basename(argv[0]);
	struct main_ctx *mc = &__mc;
	const struct action *act = NULL;
	char *con_arg = NULL;
	int i, opt, ret = -EINVAL;

	if (argc <= 1)
		usage(appname);

	while ((opt = getopt(argc, argv, CON_OPTSTR "h")) != -1) {
		switch (opt) {
		case 'F':
			mc->con = &con_file;
			con_arg = optarg;
			break;
#ifdef CONFIG_CON_USB
		case 'U':
			mc->con = &con_usb;
			con_arg = optarg;
			break;
#endif
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

	if (optind >= argc) {
		act = &actions[0];	/* Select first action by default */
	} else {
		for (i = 0; i < ARRAY_SIZE(actions); ++i) {
			if (strcasecmp(argv[optind], actions[i].name) != 0)
				continue;
			act = &actions[i];
			break;
		}
		if (!act) {
			fprintf(stderr, "Unknown action -- %s\n", argv[optind]);
			goto exit;
		}
		optind++;
	}

	mc->con_priv = malloc(mc->con->priv_sz);
	if (!mc->con_priv) {
		fprintf(stderr, "Unable to allocate memory for a connector private data\n");
		goto exit;
	}

	ret = mc->con->init(mc, con_arg);
	if (ret)
		goto exit;

	ret = act->func(mc, argc - optind, argv + optind);

	mc->con->clean(mc);

exit:
	free(mc->con_priv);

	return ret ? EXIT_FAILURE : EXIT_SUCCESS;
}

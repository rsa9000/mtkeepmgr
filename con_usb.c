/*
 * Copyright (c) 2017-2021, Sergey Ryazanov <ryazanov.s.a@gmail.com>
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <libusb.h>

#include "mtkeepmgr.h"

#define USB_MATCH_FILTER_BUSNUM		BIT(0)	/* Bus number match */
#define USB_MATCH_FILTER_DEVADDR	BIT(1)	/* Device address match */
#define USB_MATCH_FILTER_ID		BIT(2)	/* Device VID/PID match */
#define USB_MATCH_FILTER_PATH		BIT(3)	/* Path match */
#define USB_MATCH_FILTER_PATH_EXACT	BIT(4)	/* Require exact path match */

#define USB_MAX_PATHLEN			10

struct usb_match_filter {
	unsigned int mask;
	uint8_t busnum;		/* Bus number */
	uint8_t devaddr;	/* Device address */
	uint16_t vid;		/* Vendor ID */
	uint16_t pid;		/* Product ID */
	unsigned int plen;	/* Device path length */
	uint8_t path[USB_MAX_PATHLEN];	/* Sequence hub's ports */
};

static const struct dev_id {
	uint16_t vid;
	uint16_t pid;
} devs[] = {
	{0x148f, 0x7610},	/* Default ID of MT7610U */
	{0x148f, 0x761a},	/* TP-Link T2U dongle */
};

/* MediaTek specific (vendor) USB device commands */
enum usb_vend_cmd {
	USB_VENDOR_EEP_READ = 0x09,		/* Calibration data read */
};

struct usb_priv {
	struct libusb_context *ctx;
	struct libusb_device_handle *udh;
};

static int usb_parse_filter_arg(const char *str, struct usb_match_filter *f)
{
	char *__str, *s, *e, *p;
	int ret = -1;

	f->mask = 0;
	if (str[0] == '\0' || strcasecmp(str, "any") == 0)
		return 0;

	__str = strdup(str);
	if (!__str) {
		fprintf(stderr, "usbcon: unable to allocate buffer for argument(s) parsing\n");
		return -1;
	}

	s = __str;
	e = __str + strlen(__str);
	for (p = s; p < e; s = p + 1) {
		unsigned int v1, v2;
		int n, l1, l2;

		p = strchr(s, ',') ? : e;
		*p = '\0';

		n = sscanf(s, "0x%x%n:0x%x%n", &v1, &l1, &v2, &l2);
		if (n == 2 && l1 == 6 && l2 == 13) {
			f->mask = USB_MATCH_FILTER_ID;
			f->vid = v1;
			f->pid = v2;
			continue;
		}
		n = sscanf(s, "%x%n:%x%n", &v1, &l1, &v2, &l2);
		if (n == 2 && l1 == 4 && l2 == 9) {
			f->mask |= USB_MATCH_FILTER_ID;
			f->vid = v1;
			f->pid = v2;
			continue;
		}
		n = sscanf(s, "%u:%u%n", &v1, &v2, &l2);
		if (n == 2 && s[l2] == '\0' && v1 <= 0xff && v2 <= 0xff) {
			f->mask |= USB_MATCH_FILTER_BUSNUM |
				   USB_MATCH_FILTER_DEVADDR;
			f->busnum = v1;
			f->devaddr = v2;
			continue;
		}
		if (strchr(s, '/')) {
			n = sscanf(s, "%u%n", &v1, &l2);
			if (!n || s[l2] != '/' || v1 > 0xff) {
				fprintf(stderr, "usbcon: unable to parse bus number of device path -- %s\n", s);
				goto exit;
			}
			f->mask |= USB_MATCH_FILTER_BUSNUM |
				   USB_MATCH_FILTER_PATH;
			f->busnum = v1;

			f->plen = 0;
			for (s += l2 + 1; s[-1] != '\0'; s += l2 + 1) {
				if (s[0] == '\0')
					break;

				n = sscanf(s, "%u%n", &v1, &l2);
				if (!n || (s[l2] != '/' && s[l2] != '\0') ||
				    v1 > 0xff) {
					fprintf(stderr, "usbcon: unable to parse %d port in the device path -- %s\n",
						f->plen + 1, s);
					goto exit;
				} else if (f->plen >= ARRAY_SIZE(f->path)) {
					fprintf(stderr, "usbcon: too long device path, maximum allowed length is %d port elements\n",
						(int)ARRAY_SIZE(f->path));
					goto exit;
				}
				f->path[f->plen++] = v1;
			}
			if (!f->plen) {
				fprintf(stderr, "usbcon: device path should include at least one hub port\n");
				goto exit;
			}

			if (s[-1] != '/')
				f->mask |= USB_MATCH_FILTER_PATH_EXACT;

			continue;
		}

		fprintf(stderr, "usbcon: unable to parse argument token -- %s\n", s);
		goto exit;
	}

	if (f->mask & USB_MATCH_FILTER_DEVADDR &&
	    f->mask & USB_MATCH_FILTER_PATH) {
		fprintf(stderr, "usbcon: unable to use device address and device path filters simultaneously\n");
		goto exit;
	}

	ret = 0;

exit:
	free(__str);

	return ret;
}

/**
 * This routine reads calibration data that are look like EEPROM data and
 * possibly preprocessed by the chip (i.e. these data are not always raw
 * storage content). Reading perfomed using USB specific data obtaining
 * chip interface.
 */
static int usb_eep_read_block(struct main_ctx *mc, unsigned off, unsigned size,
			      uint8_t *buf)
{
	struct usb_priv *upd = mc->con_priv;
	unsigned int timeout = 300 * (size / 0x100 ? : 1); /* ms */
	const uint8_t bmRequestType = LIBUSB_ENDPOINT_IN |
				      LIBUSB_REQUEST_TYPE_VENDOR |
				      LIBUSB_RECIPIENT_DEVICE;

	return libusb_control_transfer(upd->udh, bmRequestType,
				       USB_VENDOR_EEP_READ, 0, off, buf,
				       size, timeout);
}

static int usb_eep2buf(struct main_ctx *mc)
{
#define READ_BLOCK_SZ	0x20
	int off, res;

	/**
	 * We do not know in advance the EEPROM size, so read by small blocks
	 * (of almost arbitrary size) and looking for the overlap to determine
	 * actual EEPROM size.
	 */
	for (off = 0; off + READ_BLOCK_SZ <= sizeof(mc->eep_buf);
	     off += READ_BLOCK_SZ) {
		res = usb_eep_read_block(mc, off, READ_BLOCK_SZ, &mc->eep_buf[off]);
		if (res < 0) {
			fprintf(stderr, "usbcon: unable to read EEPROM at 0x%04x: %s\n",
				off, libusb_strerror(res));
			return -1;
		} else if (res != READ_BLOCK_SZ) {
			fprintf(stderr, "usbcon: read less then requested block (%d bytes instead of %d bytes)\n",
				res, READ_BLOCK_SZ);
			return -1;
		}

		if (off) {
			res = memcmp(&mc->eep_buf[0], &mc->eep_buf[off],
				     READ_BLOCK_SZ);
			if (res == 0) {
				printf("usbcon: EEPROM overlap detected at 0x%04x\n",
				       off);
				break;
			}
		}
	}
	if (off + READ_BLOCK_SZ > sizeof(mc->eep_buf)) {
		fprintf(stderr, "usbcon: EEPROM is bigger then internal buffer, analysis will be limited by a %d bytes\n",
			off);
	}

	mc->eep_len = off;	/* Preserve detected size */

	return 0;

#undef READ_BLOCK_SZ
}

static int usb_init(struct main_ctx *mc, const char *arg_str)
{
	struct usb_priv *upd = mc->con_priv;
	struct usb_match_filter filter;
	struct libusb_device **list = NULL;
	int list_len, i, j;
	int res, ret = -EIO;

	res = usb_parse_filter_arg(arg_str, &filter);
	if (res)
		return res;

	memset(upd, 0x00, sizeof(*upd));

	res = libusb_init(&upd->ctx);
	if (res < 0) {
		fprintf(stderr, "usbcon: unable to initialize libusb context: %s\n",
			libusb_strerror(res));
		return -EIO;
	}

	list_len = libusb_get_device_list(upd->ctx, &list);
	if (list_len < 0) {
		fprintf(stderr, "usbcon: unable to obtain USB devices list: %s\n",
			libusb_strerror(list_len));
		goto error;
	}

	for (i = 0; i < list_len; ++i) {
		struct libusb_device_descriptor desc;
		uint8_t busnum, devaddr;
		int knownid;

		res = libusb_get_device_descriptor(list[i], &desc);
		if (res) {
			fprintf(stderr, "usbcon: unable to get USB device descriptor: %s\n",
				libusb_strerror(res));
			goto error;
		}
		busnum = libusb_get_bus_number(list[i]);
		devaddr = libusb_get_device_address(list[i]);

		if (filter.mask & USB_MATCH_FILTER_BUSNUM &&
		    busnum != filter.busnum)
				continue;
		if (filter.mask & USB_MATCH_FILTER_DEVADDR &&
		    devaddr != filter.devaddr)
				continue;
		if (filter.mask & USB_MATCH_FILTER_PATH) {
			uint8_t path[ARRAY_SIZE(filter.path)];
			int plen;

			plen = libusb_get_port_numbers(list[i], path,
						       ARRAY_SIZE(path));
			if (plen == LIBUSB_ERROR_OVERFLOW) {
				fprintf(stderr, "usbcon: device bus=%u,addr=%u,vid=%04x,pid=%04x has path length greater than %d elements and will be skipped\n",
					busnum, devaddr, desc.idVendor,
					desc.idProduct, (int)ARRAY_SIZE(path));
				continue;
			}

			if (plen < filter.plen)
				continue;
			if (filter.mask & USB_MATCH_FILTER_PATH_EXACT &&
			    plen != filter.plen)
				continue;

			if (memcmp(filter.path, path, filter.plen) != 0)
				continue;
		}

		/* Check against the table of known devices */
		knownid = -1;
		for (j = 0; j < ARRAY_SIZE(devs); ++j) {
			if (desc.idVendor != devs[j].vid ||
			    desc.idProduct != devs[j].pid)
				continue;
			knownid = j;
			break;
		}

		if (filter.mask & USB_MATCH_FILTER_ID) {
			if (desc.idVendor != filter.vid ||
			    desc.idProduct != filter.pid)
				continue;
			if (knownid == -1)
				fprintf(stderr, "usbcon: device bus=%u,addr=%u,vid=%04x,pid=%04x has unknown VID/PID, but match is forced by the filter\n",
						busnum, devaddr, desc.idVendor,
						desc.idProduct);
		} else if (knownid == -1) {
			if (filter.mask)	/* Have at least one filter */
				fprintf(stderr, "usbcon: device bus=%u,addr=%u,vid=%04x,pid=%04x has unknown VID/PID and will be skipped\n",
						busnum, devaddr, desc.idVendor,
						desc.idProduct);
			continue;
		}

		break;	/* Got a match, break the search loop */
	}

	if (i == list_len) {
		fprintf(stderr, "usbcon: unable to found a matched USB device\n");
		ret = -ENODEV;
		goto error;
	}

	res = libusb_open(list[i], &upd->udh);
	if (res) {
		fprintf(stderr, "usbcon: unable to open USB device: %s\n",
			libusb_strerror(res));
		goto error;
	}

	libusb_free_device_list(list, list_len);
	list = NULL;

	res = usb_eep2buf(mc);
	if (res)
		goto error;

	return 0;

error:
	if (upd->udh)
		libusb_close(upd->udh);
	if (list)
		libusb_free_device_list(list, list_len);
	libusb_exit(upd->ctx);

	return ret;
}

void usb_clean(struct main_ctx *mc)
{
	struct usb_priv *upd = mc->con_priv;

	if (upd->udh)
		libusb_close(upd->udh);
	if (upd->ctx)
		libusb_exit(upd->ctx);
}

const struct connector_desc con_usb = {
	.name = "USB",
	.priv_sz = sizeof(struct usb_priv),
	.init = usb_init,
	.clean = usb_clean,
};

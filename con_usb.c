/*
 * Copyright (c) 2017-2021, Sergey Ryazanov <ryazanov.s.a@gmail.com>
 */

#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <libusb.h>

#include "mtkeepmgr.h"

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
	struct libusb_device **list = NULL;
	int list_len, i, j;
	int res, ret = -EIO;

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

		res = libusb_get_device_descriptor(list[i], &desc);
		if (res) {
			fprintf(stderr, "usbcon: unable to get USB device descriptor: %s\n",
				libusb_strerror(res));
			goto error;
		}

		/* Check against the table of known devices */
		for (j = 0; j < ARRAY_SIZE(devs); ++j) {
			if (desc.idVendor != devs[j].vid ||
			    desc.idProduct != devs[j].pid)
				continue;
			break;
		}
		if (j == ARRAY_SIZE(devs))	/* No VID/PID match */
			continue;

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

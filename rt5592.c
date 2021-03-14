/**
 * Ralink RT5592 / MediaTek MT7592 EEPROM parser
 *
 * Copyright (c) 2021, Sergey Ryazanov <ryazanov.s.a@gmail.com>
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

#include "mtkeepmgr.h"
#include "utils.h"
#include "rt5592.h"

static int rt5592_eep_parse(struct main_ctx *mc)
{
	printf("[Device identification]\n");
	printf("  MacAddr       : %s\n", get_macaddr_str(mc));
	printf("  PCIDevID      : %04Xh\n", eep_read_word(mc, E_PCI_DEV_ID));
	printf("  PCIVenID      : %04Xh\n", eep_read_word(mc, E_PCI_VEN_ID));
	printf("  PCISubsysDevID: %04Xh\n", eep_read_word(mc, E_PCI_SUB_DEV_ID));
	printf("  PCISubsysVenID: %04Xh\n", eep_read_word(mc, E_PCI_SUB_VEN_ID));
	printf("\n");

	return 0;
}

CHIP(RT5592, 0x5592, rt5592_eep_parse);
CHIP(MT7592, 0x7592, rt5592_eep_parse);

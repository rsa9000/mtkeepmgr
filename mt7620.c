/**
 * MediaTek MT7620 EEPROM parser
 *
 * Copyright (c) 2017, Sergey Ryazanov <ryazanov.s.a@gmail.com>
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
#include "mt7620.h"

static int mt7620_eep_parse(struct main_ctx *mc)
{
	uint16_t val;

	printf("[Device identification]\n");
	printf("  MacAddr       : %s\n", get_macaddr_str(mc));
	printf("\n");

	printf("[NIC configuration]\n");
	val = eep_read_word(mc, E_NIC_CFG0);
	printf("  Cfg0          : %04Xh\n", val);
	printf("    RxPath      : %u\n", FIELD_GET(E_NIC_CFG0_RX_PATH, val));
	printf("    TxPath      : %u\n", FIELD_GET(E_NIC_CFG0_TX_PATH, val));
	val = eep_read_word(mc, E_NIC_CFG1);
	printf("  Cfg1          : %04Xh\n", val);
	printf("    Ext. TxALC  : %s\n", val & E_NIC_CFG1_EXT_TX_ALC ? "Enable" : "Disable");
	printf("    LNA 2GHz    : %s\n", val & E_NIC_CFG1_EXT_2G_LNA ? "External" : "Internal");
	printf("    40MHz 2G SB : %s\n", val & E_NIC_CFG1_40M_2G_SB ? "Enable" : "Disable");
	printf("    WPS button  : %s\n", val & E_NIC_CFG1_WPS_BUT_EN ? "Enable" : "Disable");
	printf("    40MHz 2GHz  : %s\n", val & E_NIC_CFG1_40M_2G_DIS ? "Disable" : "Enable");
	printf("    Ext. LNA    : %s\n", val & E_NIC_CFG1_EXT_LNA ? "True" : "False");
	printf("    Int. TxALC  : %s\n", val & E_NIC_CFG1_INT_TX_ALC ? "True" : "False");
	printf("    Tx0 PA      : %s\n", val & E_NIC_CFG1_TX0_EXT_PA ? "Enternal" : "Internal");
	printf("    Tx1 PA      : %s\n", val & E_NIC_CFG1_TX1_EXT_PA ? "Enternal" : "Internal");
	val = eep_read_word(mc, E_NIC_CFG2);
	printf("  Cfg2          : %04Xh\n", val);
	printf("    RxStream    : %u\n", FIELD_GET(E_NIC_CFG2_RX_STREAM, val));
	printf("    TxStream    : %u\n", FIELD_GET(E_NIC_CFG2_TX_STREAM, val));
	printf("    RxTempComp. : %s\n", val & E_NIC_CFG2_RXTEMP_C_DIS ? "Disable" : "Enable");
	printf("\n");

	return 0;
}

CHIP(MT7620, 0x7620, mt7620_eep_parse);

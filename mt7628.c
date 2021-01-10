/**
 * MediaTek MT7628 EEPROM parser
 *
 * Copyright (c) 2020, Sergey Ryazanov <ryazanov.s.a@gmail.com>
 */

#include <stdio.h>

#include "mtkeepmgr.h"
#include "utils.h"
#include "mt7628.h"

static int mt7628_eep_parse(struct main_ctx *mc)
{
	printf("[Device identification]\n");
	printf("  MacAddr       : %s\n", get_macaddr_str(mc));
	printf("\n");

	return 0;
}

CHIP(MT7628, 0x7628, mt7628_eep_parse);

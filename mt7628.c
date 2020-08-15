/**
 * MediaTek MT7628 EEPROM parser
 *
 * Copyright (c) 2020, Sergey Ryazanov <ryazanov.s.a@gmail.com>
 */

#include <stdio.h>

#include "medump.h"
#include "utils.h"
#include "mt7628.h"

static int mt7628_eep_parse(void)
{
	printf("[Device identification]\n");
	printf("  MacAddr       : %s\n", get_macaddr_str());
	printf("\n");

	return 0;
}

CHIP(MT7628, 0x7628, mt7628_eep_parse);

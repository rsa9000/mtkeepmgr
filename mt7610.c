/**
 * MediaTek MT7610 EEPROM parser
 *
 * Copyright (c) 2016, Sergey Ryazanov <ryazanov.s.a@gmail.com>
 */

#include <stdio.h>

#include "medump.h"
#include "utils.h"
#include "mt7610.h"

static int mt7610_eep_parse(void)
{
	printf("[Device identification]\n");
	printf("  MacAddr       : %s\n", get_macaddr_str());
	printf("  PCIDevID      : 0x%04x\n", eep_read_word(E_PCI_DEV_ID));
	printf("  PCIVenID      : 0x%04x\n", eep_read_word(E_PCI_VEN_ID));
	printf("  PCISubsysDevID: 0x%04x\n", eep_read_word(E_PCI_SUB_DEV_ID));
	printf("  PCISubsysVenID: 0x%04x\n", eep_read_word(E_PCI_SUB_VEN_ID));
	printf("\n");

	return 0;
}

CHIP(MT7610, 0x7610, mt7610_eep_parse);

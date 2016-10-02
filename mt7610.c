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
	uint16_t val;

	printf("[Device identification]\n");
	printf("  MacAddr       : %s\n", get_macaddr_str());
	printf("  PCIDevID      : 0x%04x\n", eep_read_word(E_PCI_DEV_ID));
	printf("  PCIVenID      : 0x%04x\n", eep_read_word(E_PCI_VEN_ID));
	printf("  PCISubsysDevID: 0x%04x\n", eep_read_word(E_PCI_SUB_DEV_ID));
	printf("  PCISubsysVenID: 0x%04x\n", eep_read_word(E_PCI_SUB_VEN_ID));
	printf("\n");

	printf("[Device configuration]\n");
	printf("  CMB aux option: 0x%04x\n", eep_read_word(E_CMB_AUX_OPT));
	printf("  XTAL opt???   : 0x%04x\n", eep_read_word(E_XTAL_OPT));
	val = eep_read_word(E_NIC_CFG0);
	printf("  RxPath        : %u\n", FIELD_GET(E_NIC_CFG0_RX_PATH, val));
	printf("  TxPath        : %u\n", FIELD_GET(E_NIC_CFG0_TX_PATH, val));
	printf("  PA 2GHz       : %s\n", val & E_NIC_CFG0_INT_2G_PA ? "Internal" : "External");
	printf("  PA 5GHz       : %s\n", val & E_NIC_CFG0_INT_5G_PA ? "Internal" : "External");
	printf("  PA current    : %u ma\n", val & E_NIC_CFG0_EXT_PA_CURR ? 8 : 16);
	val = eep_read_word(E_NIC_CFG1);
	printf("  RF Ctrl       : %s\n", val & E_NIC_CFG1_HW_RF_CTRL ? "Hw" : "Driver");
	printf("  DynTxAgcCtrl  : %s\n", val & E_NIC_CFG1_DYN_TX_AGC ? "Enable" : "Disable");
	printf("  LNA 2GHz      : %s\n", val & E_NIC_CFG1_EXT_2G_LNA ? "External" : "Internal");
	printf("  LNA 5GHz      : %s\n", val & E_NIC_CFG1_EXT_5G_LNA ? "External" : "Internal");
	printf("  CardBus Accel : %s\n", val & E_NIC_CFG1_CB_ACCEL_DIS ? "Disable" : "Enable");
	printf("  40MHZ 2GHz SB : %s\n", val & E_NIC_CFG1_40M_2G_SB ? "Enable" : "Disable");
	printf("  40MHZ 5GHz SB : %s\n", val & E_NIC_CFG1_40M_5G_SB ? "Enable" : "Disable");
	printf("  WPS button    : %s\n", val & E_NIC_CFG1_WPS_BUT_EN ? "Enable" : "Disable");
	printf("  40MHz 2GHz    : %s\n", val & E_NIC_CFG1_40M_2G_DIS ? "Disable" : "Enable");
	printf("  40MHz 5GHz    : %s\n", val & E_NIC_CFG1_40M_5G_DIS ? "Disable" : "Enable");
	printf("  Ant. diversity: %s\n", val & E_NIC_CFG1_ANT_DIV ? "True" : "False");
	printf("  Ant. opt.     : %s\n", val & E_NIC_CFG1_ANT_OPT ? "True" : "False");
	printf("  Internal TxALC: %s\n", val & E_NIC_CFG1_INT_TX_ALC ? "True" : "False");
	printf("  Coexistance   : %s\n", val & E_NIC_CFG1_COEX ? "True" : "False");
	val = eep_read_word(E_NIC_CFG2);
	printf("  RxStream      : %u\n", FIELD_GET(E_NIC_CFG2_RX_STREAM, val));
	printf("  TxStream      : %u\n", FIELD_GET(E_NIC_CFG2_TX_STREAM, val));
	printf("  CoexAnt       : %s\n", val & E_NIC_CFG2_COEX_ANT ? "True" : "False");
	printf("  XtalOpt       : %u\n", FIELD_GET(E_NIC_CFG2_XTAL_OPT, val));
	printf("  RxTempCompens.: %s\n", val & E_NIC_CFG2_RXTEMP_C_DIS ? "Disable" : "Enable");
	printf("\n");

	return 0;
}

CHIP(MT7610, 0x7610, mt7610_eep_parse);

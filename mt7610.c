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
	printf("  CalibInFlash  : %s\n", val & E_NIC_CFG2_CAL_IN_FLASH ? "True" : "False");
	printf("  XtalOpt       : %u\n", FIELD_GET(E_NIC_CFG2_XTAL_OPT, val));
	printf("  RxTempCompens.: %s\n", val & E_NIC_CFG2_RXTEMP_C_DIS ? "Disable" : "Enable");
	val = eep_read_word(E_FREQ_OFFSET);
	printf("  FreqOffset    : 0x%02x\n", FIELD_GET(E_FREQ_OFFSET_FO, val));
	printf("\n");

	printf("[Country region code]\n");
	val = eep_read_word(E_COUNTRY_REGION);
	if (FIELD_GET(E_COUNTRY_REGION_5G, val) == 0xff)
		printf("  5GHz country  : <none>\n");
	else
		printf("  5GHz country  : %u\n", FIELD_GET(E_COUNTRY_REGION_5G, val));
	if (FIELD_GET(E_COUNTRY_REGION_2G, val) == 0xff)
		printf("  2GHz country  : <none>\n");
	else
		printf("  2GHz country  : %u\n", FIELD_GET(E_COUNTRY_REGION_2G, val));
	printf("\n");

	printf("[External LNA gain]\n");
	val = eep_read_word(E_LNA_GAIN_0);
	printf("  2GHz (1-14)   : %u dB\n", FIELD_GET(E_LNA_GAIN_2G, val));
	printf("  5GHz (36-46)  : %u dB\n", FIELD_GET(E_LNA_GAIN_5G_0, val));
	val = eep_read_word(E_LNA_GAIN_1);
	printf("  5GHz (100-128): %u dB\n", FIELD_GET(E_LNA_GAIN_5G_1, val));
	val = eep_read_word(E_LNA_GAIN_2);
	printf("  5GHz (132-165): %u dB\n", FIELD_GET(E_LNA_GAIN_5G_2, val));
	printf("\n");

	printf("[BBP RSSI offsets]\n");
	val = eep_read_word(E_RSSI_OFFSET_2G);
	printf("  2GHz Offset0  : %d dB\n", (int8_t)FIELD_GET(E_RSSI_OFFSET_2G_0, val));
	printf("  2GHz Offset1  : %d dB\n", (int8_t)FIELD_GET(E_RSSI_OFFSET_2G_1, val));
	val = eep_read_word(E_RSSI_OFFSET_5G);
	printf("  5GHz Offset0  : %d dB\n", (int8_t)FIELD_GET(E_RSSI_OFFSET_5G_0, val));
	printf("  5GHz Offset1  : %d dB\n", (int8_t)FIELD_GET(E_RSSI_OFFSET_5G_1, val));
	printf("\n");

	return 0;
}

CHIP(MT7610, 0x7610, mt7610_eep_parse);

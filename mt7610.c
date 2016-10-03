/**
 * MediaTek MT7610 EEPROM parser
 *
 * Copyright (c) 2016, Sergey Ryazanov <ryazanov.s.a@gmail.com>
 */

#include <stdio.h>

#include "medump.h"
#include "utils.h"
#include "mt7610.h"

/* Return power delta in 0.5 dBm step */
static int pwr_delta_unpack(const uint8_t val)
{
	if (!(val & E_PWR_DELTA_EN))
		return 0;

	return val & E_PWR_DELTA_SIGN ? FIELD_GET(E_PWR_DELTA_VAL, val):
					-1 * FIELD_GET(E_PWR_DELTA_VAL, val);
}

static const char *pwr_delta_str(const uint8_t val)
{
	static char buf[0x10];
	int delta;

	if (0xff == val)
		return "0 dBm (default)";

	delta = pwr_delta_unpack(val);
	snprintf(buf, sizeof(buf), "%.1f dBm", (double)delta / 2);

	return buf;
}

static void mt7610_dump_channel_power(void)
{
	static const unsigned ch_2gh[] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11,
					  12, 13, 14};
	static const unsigned ch_5gh_0[] = {36, 38, 40, 44, 46, 48, 52, 54, 56,
					    60, 62, 64};
	static const unsigned ch_5gh_1[] = {100, 102, 104, 108, 110, 112, 116,
					    118, 120, 124, 126, 128, 132, 134,
					    136, 140};
	static const unsigned ch_5gh_2[] = {149, 151, 153, 157, 159, 161, 165,
					    167, 169, 171, 173};
	static const struct {
		const char *name;	/* Subband name */
		unsigned ee_base;	/* EEPROM base offset */
		unsigned const *ch;	/* Channels array */
		unsigned nchan;		/* Number of channels */
	} *sb, subbands[] = {
		{
			.name = "2.4 GHz",
			.ee_base = E_CH_PWR_2G_BASE,
			.ch = ch_2gh,
			.nchan = sizeof(ch_2gh)/sizeof(ch_2gh[0]),
		}, {
			.name = "5 GHz (low)",
			.ee_base = E_CH_PWR_5G_0_BASE,
			.ch = ch_5gh_0,
			.nchan = sizeof(ch_5gh_0)/sizeof(ch_5gh_0[0]),
		}, {
			.name = "5 GHz (middle)",
			.ee_base = E_CH_PWR_5G_1_BASE,
			.ch = ch_5gh_1,
			.nchan = sizeof(ch_5gh_1)/sizeof(ch_5gh_1[0]),
		}, {
			.name = "5 GHz (hight)",
			.ee_base = E_CH_PWR_5G_2_BASE,
			.ch = ch_5gh_2,
			.nchan = sizeof(ch_5gh_2)/sizeof(ch_5gh_2[0]),
		}
	};
	unsigned pwr[0x10];	/* size = MAX(2G, 5G0, 5G1, 5G2) */
	unsigned si, ci;
	uint16_t eeval;

	for (si = 0; si < sizeof(subbands)/sizeof(subbands[0]); ++si) {
		sb = &subbands[si];
		printf("  Subband: %s\n", sb->name);
		for (ci = 0; ci < sb->nchan; ci += 2) {
			eeval = eep_read_word(sb->ee_base + ci);
			pwr[ci + 0] = FIELD_GET(E_CH_PWR_LO, eeval);
			pwr[ci + 1] = FIELD_GET(E_CH_PWR_HI, eeval);
		}
		printf("  Channel: ");
		for (ci = 0; ci < sb->nchan; ++ci)
			printf(" %3u", sb->ch[ci]);
		printf("\n");
		printf("  Power  : ");
		for (ci = 0; ci < sb->nchan; ++ci)
			printf(" %3u", pwr[ci] > E_CH_PWR_MAX ?
				       E_CH_PWR_DEFAULT : pwr[ci]);
		printf("\n");
	}
}

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
	val = eep_read_word(E_TEMP_OFFSET);
	printf("  TempOffset    : %d\n", (int8_t)FIELD_GET(E_TEMP_OFFSET_VAL, val));
	val = eep_read_word(E_5G_SUBBANDS);
	printf("  5GHz mid chan : %u\n", FIELD_GET(E_5G_SUBBANDS_MID_CH, val));
	printf("  5GHz higt chan: %u\n", FIELD_GET(E_5G_SUBBANDS_HIG_CH, val));
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

	printf("[Tx power delta]\n");
	val = eep_read_word(E_40M_PWR_DELTA);
	printf("  2GHz 40MHz    : %s\n",
		pwr_delta_str(FIELD_GET(E_40M_PWR_DELTA_2G, val)));
	printf("  5GHz 40MHz    : %s\n",
	       pwr_delta_str(FIELD_GET(E_40M_PWR_DELTA_5G, val)));
	val = eep_read_word(E_80M_PWR_DELTA);
	printf("  5GHz 80MHz    : %s\n",
	       pwr_delta_str(FIELD_GET(E_80M_PWR_DELTA_5G, val)));
	printf("\n");

	printf("[Per channel power table]\n");
	mt7610_dump_channel_power();
	printf("\n");

	return 0;
}

CHIP(MT7610, 0x7610, mt7610_eep_parse);

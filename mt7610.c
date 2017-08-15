/**
 * MediaTek MT7610 EEPROM parser
 *
 * Copyright (c) 2016, Sergey Ryazanov <ryazanov.s.a@gmail.com>
 */

#include <stdio.h>
#include <string.h>

#include "medump.h"
#include "utils.h"
#include "mt7610.h"

/* Preserved values for further calculations */
static int8_t temp_offset;		/* Temperature offset */

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

/* Return power delta in 0.5 dBm step */
static int pwr_rate_unpack(const uint8_t val)
{
	return val & E_RATE_PWR_SIGN ? FIELD_GET(E_RATE_PWR_VAL, val) - 32:
				       FIELD_GET(E_RATE_PWR_VAL, val);
}

static const char *pwr_rate_str(const uint8_t val)
{
	static char buf[0x10];
	int pwr = pwr_rate_unpack(val);

	snprintf(buf, sizeof(buf), "%.1f", (double)pwr / 2);

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

static void mt7610_dump_rate_power(void)
{
	static const char *blocks[3] = {"2.4GHz", "5GHz", "STBC"};
	static const struct {
		const char *title_lo;
		const char *title_hi;
		unsigned off[3];	/* 2GHz, 5GHz, STBC */
	} *r, rates[] = {
		{
			"CCK 1M/2M", "CCK 5.5M/11M",
			{E_RATE_PWR_2G_CCK_1_55, 0, 0}
		}, {
			"OFDM 6M/9M", "OFDM 12M/18M",
			{E_RATE_PWR_2G_OFDM_6_12, E_RATE_PWR_5G_OFDM_6_12, 0},
		}, {
			"OFDM 24M/36M", "OFDM 48M/54M",
			{E_RATE_PWR_2G_OFDM_24_48, E_RATE_PWR_5G_OFDM_24_48, 0},
		}, {
			"HT/VHT MCS 0/1", "HT/VHT MCS 2/3",
			{E_RATE_PWR_2G_MCS_0_2, E_RATE_PWR_5G_MCS_0_2, E_RATE_PWR_STBC_MCS_0_2}
		}, {
			"HT/VHT MCS 4/5", "HT/VHT MCS 6/7",
			{E_RATE_PWR_2G_MCS_4_6, E_RATE_PWR_5G_MCS_4_6, E_RATE_PWR_STBC_MCS_4_6}
		}, {
			"VHT MCS8", "VHT MCS9",
			{0, E_RATE_PWR_5G_VHT_8_9, 0}
		}, {
			NULL, NULL
		}
	};
	uint16_t val[3];
	unsigned i;

	printf("                   ");
	for (i = 0; i < 3; ++i)
		printf("%10s", blocks[i]);
	printf("\n");

	for (r = rates; r->title_lo || r->title_hi; ++r) {
		for (i = 0; i < 3; ++i)
			if (r->off[i])
				val[i] = eep_read_word(r->off[i]);
		if (r->title_lo) {
			printf("  %-16s:", r->title_lo);
			for (i = 0; i < 3; ++i)
				printf("%10s", r->off[i] ? pwr_rate_str(FIELD_GET(E_RATE_PWR_LO, val[i])) : "");
			printf("\n");
		}
		if (r->title_hi) {
			printf("  %-16s:", r->title_hi);
			for (i = 0; i < 3; ++i)
				printf("%10s", r->off[i] ? pwr_rate_str(FIELD_GET(E_RATE_PWR_HI, val[i])) : "");
			printf("\n");
		}
	}
}

static void mt7610_read_tssi_tcomp_tbl(unsigned off, int8_t *tbl)
{
	uint16_t val;
	int i;

	/* Read data from eeprom */
	for (i = 0; i < E_TSSI_TCOMP_N / 2; ++i) {
		val = eep_read_word(off + 2 * i);
		tbl[2 * i + 0] = (val >> 8) & 0xff;
		tbl[2 * i + 1] = (val >> 0) & 0xff;
	}

	/* Place neutral element in the middle of the table */
	memmove(&tbl[E_TSSI_TCOMP_N / 2 + 1], &tbl[E_TSSI_TCOMP_N / 2],
		E_TSSI_TCOMP_N / 2);
	tbl[E_TSSI_TCOMP_N / 2] = 0;
}

static void mt7610_adj_tssi_tcomp_tbl(int8_t *tbl)
{
	int i, tmp;

	for (i = 0; i < E_TSSI_TCOMP_N + 1; ++i) {
		tmp = (int)tbl[i] + temp_offset;
		if (tmp < E_TSSI_TCOMP_VAL_MIN)
			tbl[i] = E_TSSI_TCOMP_VAL_MIN;
		else if (tmp > E_TSSI_TCOMP_VAL_MAX)
			tbl[i] = E_TSSI_TCOMP_VAL_MAX;
		else
			tbl[i] = tmp;
	}
}

static const char *mt7610_dump_tssi_tcomp_tbl(int8_t *tbl)
{
	static char buf[0x80];
	char *p = buf, *e = buf + sizeof(buf);
	unsigned i;

	for (i = 0; i < E_TSSI_TCOMP_N + 1; ++i)
		p += snprintf(p, e - p, " %+4d", tbl[i]);

	return &buf[1];
}

static const char *mt7610_dump_tssi_tcomp(unsigned off)
{
	int8_t tbl[E_TSSI_TCOMP_N + 1];	/* Number of points + neutral */

	mt7610_read_tssi_tcomp_tbl(off, tbl);
	mt7610_adj_tssi_tcomp_tbl(tbl);

	return mt7610_dump_tssi_tcomp_tbl(tbl);
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
	temp_offset = (int8_t)FIELD_GET(E_TEMP_OFFSET_VAL, val);
	printf("  TempOffset    : %d\n", temp_offset);
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

	printf("[Per rate power table]\n");
	mt7610_dump_rate_power();
	printf("\n");

	printf("[TSSI temperature compensation]\n");
	val = eep_read_word(E_TX_AGC_STEP);
	if (FIELD_GET(E_TX_AGC_STEP_VAL, val) == 0xff)
		printf("  Tx AGC step   : 1.0 dBm (default)\n");
	else
		printf("  Tx AGC step   : %u.%u dBm\n",
		       FIELD_GET(E_TX_AGC_STEP_VAL, val) / 2,
		       FIELD_GET(E_TX_AGC_STEP_VAL, val) & 1 ? 5 : 0);
	val = eep_read_word(E_TSSI_TCOMP_5G_BOUND);
	printf("  5GHz boundary : %u (channel)\n", FIELD_GET(E_TSSI_TCOMP_5G_BOUND_VAL, val));
	printf("  5GHz group 1  : {%s}\n",
	       mt7610_dump_tssi_tcomp(E_TSSI_TCOMP_5G_1_BASE));
	printf("  5GHz group 2  : {%s}\n",
	       mt7610_dump_tssi_tcomp(E_TSSI_TCOMP_5G_2_BASE));
	printf("\n");

	return 0;
}

CHIP(MT7610, 0x7610, mt7610_eep_parse);

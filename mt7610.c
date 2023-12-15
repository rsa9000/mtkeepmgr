/**
 * MediaTek MT7610 EEPROM parser
 *
 * Copyright (c) 2016-2017, Sergey Ryazanov <ryazanov.s.a@gmail.com>
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
#include <string.h>

#include "mtkeepmgr.h"
#include "utils.h"
#include "mt7610.h"

/* Preserved values for further calculations */
static int8_t temp_offset;		/* Temperature offset */

static const char *pwr_chan_str(const uint8_t val)
{
	static char buf[0x10];
	unsigned __val;

	if (E_CH_PWR_MIN <= val && val <= E_CH_PWR_MAX)
		__val = val;
	else
		__val = E_CH_PWR_DEFAULT;

	/* Value is in 0.5 dBm and non-negative */
	snprintf(buf, sizeof(buf), "%.1f", (double)__val / 2);

	return buf;

}

static const char *pwr_target_str(const uint8_t val)
{
	static char buf[0x20];
	char str[0x12];

	if (0x00 == val || 0xff == val)
		snprintf(str, sizeof(str), "16.0 dBm, default");
	else
		snprintf(str, sizeof(str), "%.1f dBm", (double)val / 2);

	snprintf(buf, sizeof(buf), "%02Xh (%s)", val, str);

	return buf;
}

/* Return decoded power delta */
static const char *pwr_delta_str(const uint8_t val)
{
	static char buf[0x20];
	char str[0x12];
	int delta;

	if (0xff == val) {
		snprintf(str, sizeof(str), "0.0 dBm, default");
	} else if (!(val & E_PWR_DELTA_EN)) {
		snprintf(str, sizeof(str), "0.0 dBm, disabled");
	} else {
		delta = val & E_PWR_DELTA_SIGN ?
			FIELD_GET(E_PWR_DELTA_VAL, val):
			-1 * FIELD_GET(E_PWR_DELTA_VAL, val);
		snprintf(str, sizeof(str), "%+.1f dBm", (double)delta / 2);
	}

	snprintf(buf, sizeof(buf), "%02Xh (%s)", val, str);

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

static const char *country_str(const uint8_t val)
{
	static char buf[0x10];

	if (val == E_COUNTRY_NONE)
		snprintf(buf, sizeof(buf), "%02Xh (<none>)", val);
	else if (val < E_COUNTRY_CUSTOM)
		snprintf(buf, sizeof(buf), "%02Xh (#%u)", val, val);
	else if (val == E_COUNTRY_CUSTOM)
		snprintf(buf, sizeof(buf), "%02Xh (<custom>)", val);
	else
		snprintf(buf, sizeof(buf), "%02Xh (<invalid>)", val);

	return buf;
}

static const char *lna_gain_str(const uint8_t val)
{
	static char buf[0x10];

	snprintf(buf, sizeof(buf), "%02Xh (%u dB)", val, val);

	return buf;
}

static const char *boundary_ch_str(const uint8_t val, const uint8_t def)
{
	static char buf[0x20];
	uint8_t ch = val == 0xff ? def : val;

	snprintf(buf, sizeof(buf), "%02Xh (%u%s)", val, ch,
		 val == 0xff ? ", default" : "");

	return buf;
}

static const char *rssi_offset_str(const uint8_t val)
{
	static char buf[0x10];

	snprintf(buf, sizeof(buf), "%02Xh (%d dB)", val, (int8_t)val);

	return buf;
}

static void mt7610_dump_channel_power(struct main_ctx *mc)
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
			.nchan = ARRAY_SIZE(ch_2gh),
		}, {
			.name = "5 GHz (low)",
			.ee_base = E_CH_PWR_5G_0_BASE,
			.ch = ch_5gh_0,
			.nchan = ARRAY_SIZE(ch_5gh_0),
		}, {
			.name = "5 GHz (middle)",
			.ee_base = E_CH_PWR_5G_1_BASE,
			.ch = ch_5gh_1,
			.nchan = ARRAY_SIZE(ch_5gh_1),
		}, {
			.name = "5 GHz (hight)",
			.ee_base = E_CH_PWR_5G_2_BASE,
			.ch = ch_5gh_2,
			.nchan = ARRAY_SIZE(ch_5gh_2),
		}
	};
	unsigned pwr[0x10];	/* size = MAX(2G, 5G0, 5G1, 5G2) */
	unsigned si, ci;
	uint16_t eeval;

	for (si = 0; si < ARRAY_SIZE(subbands); ++si) {
		sb = &subbands[si];
		printf("  Subband: %s\n", sb->name);
		for (ci = 0; ci < sb->nchan; ci += 2) {
			eeval = eep_read_word(mc, sb->ee_base + ci);
			pwr[ci + 0] = FIELD_GET(E_CH_PWR_LO, eeval);
			pwr[ci + 1] = FIELD_GET(E_CH_PWR_HI, eeval);
		}
		printf("  Channel: ");
		for (ci = 0; ci < sb->nchan; ++ci)
			printf(" %4u", sb->ch[ci]);
		printf("\n");
		printf("  Raw    : ");
		for (ci = 0; ci < sb->nchan; ++ci)
			printf("  %02Xh", pwr[ci]);
		printf("\n");
		printf("  Pwr,dBm: ");
		for (ci = 0; ci < sb->nchan; ++ci)
			printf(" %4s", pwr_chan_str(pwr[ci]));
		printf("\n");
	}
}

static void mt7610_dump_rate_power(struct main_ctx *mc)
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
			"VHT MCS 8/9", NULL,
			{0, E_RATE_PWR_5G_VHT_8_9, 0}
		}, {
			NULL, NULL
		}
	};
	uint16_t val[3];
	unsigned i;

	printf("                  ");
	printf(".---------- Raw --------.");
	printf(".--------- Power, dBm --------.");
	printf("\n");
	printf("                  |");
	for (i = 0; i < 3; ++i)
		printf("%7s|", blocks[i]);
	printf("|");
	for (i = 0; i < 3; ++i)
		printf("%9s|", blocks[i]);
	printf("\n");

	for (r = rates; r->title_lo || r->title_hi; ++r) {
		for (i = 0; i < 3; ++i)
			if (r->off[i])
				val[i] = eep_read_word(mc, r->off[i]);
		if (r->title_lo) {
			printf("  %-16s:", r->title_lo);
			for (i = 0; i < 3; ++i) {
				if (r->off[i])
					printf("    %02Xh ", FIELD_GET(E_RATE_PWR_LO, val[i]));
				else
					printf("        ");
			}
			printf(" ");
			for (i = 0; i < 3; ++i)
				printf("%9s ", r->off[i] ? pwr_rate_str(FIELD_GET(E_RATE_PWR_LO, val[i])) : "");
			printf("\n");
		}
		if (r->title_hi) {
			printf("  %-16s:", r->title_hi);
			for (i = 0; i < 3; ++i) {
				if (r->off[i])
					printf("    %02Xh ", FIELD_GET(E_RATE_PWR_HI, val[i]));
				else
					printf("        ");
			}
			printf(" ");
			for (i = 0; i < 3; ++i)
				printf("%9s ", r->off[i] ? pwr_rate_str(FIELD_GET(E_RATE_PWR_HI, val[i])) : "");
			printf("\n");
		}
	}
}

static void mt7610_read_tssi_tcomp_tbl(struct main_ctx *mc,
				       unsigned off, int8_t *tbl)
{
	uint16_t val;
	int i;

	/* Read data from eeprom */
	for (i = 0; i < E_TSSI_TCOMP_N / 2; ++i) {
		val = eep_read_word(mc, off + 2 * i);
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

static const char *mt7610_dump_tssi_tcomp(struct main_ctx *mc, unsigned off)
{
	int8_t tbl[E_TSSI_TCOMP_N + 1];	/* Number of points + neutral */

	mt7610_read_tssi_tcomp_tbl(mc, off, tbl);
	mt7610_adj_tssi_tcomp_tbl(tbl);

	return mt7610_dump_tssi_tcomp_tbl(tbl);
}

static int mt7610_eep_parse(struct main_ctx *mc)
{
	static const char *ant_div_str[] = {
		[E_ANT_DIV_DIS] = "No diversity",
		[E_ANT_DIV_EN] = "Diversity",
		[E_ANT_DIV_FIX_MAIN] = "Fixed main antenna",
		[E_ANT_DIV_FIX_AUX] = "Fixed aux antenna"
	};
	uint16_t val;

	printf("[Device identification]\n");
	printf("  MacAddr       : %s\n", get_macaddr_str(mc));
	printf("  PCIDevID      : %04Xh\n", eep_read_word(mc, E_PCI_DEV_ID));
	printf("  PCIVenID      : %04Xh\n", eep_read_word(mc, E_PCI_VEN_ID));
	printf("  PCISubsysDevID: %04Xh\n", eep_read_word(mc, E_PCI_SUB_DEV_ID));
	printf("  PCISubsysVenID: %04Xh\n", eep_read_word(mc, E_PCI_SUB_VEN_ID));
	printf("  USB Vendor ID : %04Xh\n", eep_read_word(mc, E_USB_VID));
	printf("  USB Product ID: %04Xh\n", eep_read_word(mc, E_USB_PID));
	printf("\n");

	printf("[ASIC data]\n");
	printf("  CMB aux option: %04Xh\n", eep_read_word(mc, E_CMB_AUX_OPT));
	printf("  XTAL opt???   : %04Xh\n", eep_read_word(mc, E_XTAL_OPT));
	printf("\n");

	printf("[NIC configuration]\n");
	val = eep_read_word(mc, E_NIC_CFG0);
	printf("  Cfg0          : %04Xh\n", val);
	printf("    RxPath      : %u\n", FIELD_GET(E_NIC_CFG0_RX_PATH, val));
	printf("    TxPath      : %u\n", FIELD_GET(E_NIC_CFG0_TX_PATH, val));
	printf("    PA 2GHz     : %s\n", val & E_NIC_CFG0_INT_2G_PA ? "Internal" : "External");
	printf("    PA 5GHz     : %s\n", val & E_NIC_CFG0_INT_5G_PA ? "Internal" : "External");
	printf("    PA current  : %u ma\n", val & E_NIC_CFG0_EXT_PA_CURR ? 8 : 16);
	val = eep_read_word(mc, E_NIC_CFG1);
	printf("  Cfg1          : %04Xh\n", val);
	printf("    RF Ctrl     : %s\n", val & E_NIC_CFG1_HW_RF_CTRL ? "Hardware" : "Driver");
	printf("    Ext. TxALC  : %s\n", val & E_NIC_CFG1_EXT_TX_ALC ? "Enable" : "Disable");
	printf("    LNA 2GHz    : %s\n", val & E_NIC_CFG1_EXT_2G_LNA ? "External" : "Internal");
	printf("    LNA 5GHz    : %s\n", val & E_NIC_CFG1_EXT_5G_LNA ? "External" : "Internal");
	printf("    CardBus Acc.: %s\n", val & E_NIC_CFG1_CB_ACCEL_DIS ? "Disable" : "Enable");
	printf("    40MHz 2G SB : %s\n", val & E_NIC_CFG1_40M_2G_SB ? "Enable" : "Disable");
	printf("    40MHz 5G SB : %s\n", val & E_NIC_CFG1_40M_5G_SB ? "Enable" : "Disable");
	printf("    WPS button  : %s\n", val & E_NIC_CFG1_WPS_BUT_EN ? "Enable" : "Disable");
	printf("    40MHz 2GHz  : %s\n", val & E_NIC_CFG1_40M_2G_DIS ? "Disable" : "Enable");
	printf("    40MHz 5GHz  : %s\n", val & E_NIC_CFG1_40M_5G_DIS ? "Disable" : "Enable");
	printf("    Ant. divers.: %s\n", ant_div_str[FIELD_GET(E_NIC_CFG1_ANT_DIV, val)]);
	printf("    Int. TxALC  : %s\n", val & E_NIC_CFG1_INT_TX_ALC ? "True" : "False");
	printf("    Coexistance : %s\n", val & E_NIC_CFG1_COEX ? "True" : "False");
	printf("    DAC test    : %s\n", val & E_NIC_CFG1_DAC_TEST ? "True" : "False");
	val = eep_read_word(mc, E_NIC_CFG2);
	printf("  Cfg2          : %04Xh\n", val);
	printf("    RxStream    : %u\n", FIELD_GET(E_NIC_CFG2_RX_STREAM, val));
	printf("    TxStream    : %u\n", FIELD_GET(E_NIC_CFG2_TX_STREAM, val));
	printf("    CoexAnt     : %s\n", val & E_NIC_CFG2_COEX_ANT ? "True" : "False");
	printf("    XtalOpt     : %u\n", FIELD_GET(E_NIC_CFG2_XTAL_OPT, val));
	printf("    RxTempComp. : %s\n", val & E_NIC_CFG2_RXTEMP_C_DIS ? "Disable" : "Enable");
	printf("    CalibInFlash: %s\n", val & E_NIC_CFG2_CAL_IN_FLASH ? "True" : "False");
	printf("\n");

	printf("[Misc params]\n");
	val = eep_read_word(mc, E_FREQ_OFFSET);
	printf("  FreqOffset    : %02Xh\n", FIELD_GET(E_FREQ_OFFSET_FO, val));
	val = eep_read_word(mc, E_TEMP_2G_TGT_PWR);
	temp_offset = (int8_t)FIELD_GET(E_TEMP_VAL, val);
	printf("  TempOffset    : %d\n", temp_offset);
	printf("\n");

	printf("[Country region code]\n");
	val = eep_read_word(mc, E_COUNTRY_REGION);
	printf("  2GHz country  : %s\n", country_str(FIELD_GET(E_COUNTRY_REGION_2G, val)));
	printf("  5GHz country  : %s\n", country_str(FIELD_GET(E_COUNTRY_REGION_5G, val)));
	printf("\n");

	printf("[External LNA gain]\n");
	val = eep_read_word(mc, E_LNA_GAIN_0);
	printf("  2GHz (1-14)   : %s\n",
	       lna_gain_str(FIELD_GET(E_LNA_GAIN_2G, val)));
	printf("  5GHz (36-64)  : %s\n",
	       lna_gain_str(FIELD_GET(E_LNA_GAIN_5G_0, val)));
	val = eep_read_word(mc, E_LNA_GAIN_1);
	printf("  5GHz (100-128): %s\n",
	       lna_gain_str(FIELD_GET(E_LNA_GAIN_5G_1, val)));
	val = eep_read_word(mc, E_LNA_GAIN_2);
	printf("  5GHz (132-165): %s\n",
	       lna_gain_str(FIELD_GET(E_LNA_GAIN_5G_2, val)));
	val = eep_read_word(mc, E_LNA_5G_SUBBANDS);
	printf("  5GHz mid chan : %s\n",
	       boundary_ch_str(FIELD_GET(E_LNA_5G_SUBBANDS_MID_CH, val), 100));
	printf("  5GHz higt chan: %s\n",
	       boundary_ch_str(FIELD_GET(E_LNA_5G_SUBBANDS_HIG_CH, val), 155));
	printf("\n");

	printf("[BBP RSSI offsets]\n");
	val = eep_read_word(mc, E_RSSI_OFFSET_2G);
	printf("  2GHz Offset0  : %s\n",
	       rssi_offset_str(FIELD_GET(E_RSSI_OFFSET_2G_0, val)));
	printf("  2GHz Offset1  : %s\n",
	       rssi_offset_str(FIELD_GET(E_RSSI_OFFSET_2G_1, val)));
	val = eep_read_word(mc, E_RSSI_OFFSET_5G);
	printf("  5GHz Offset0  : %s\n",
	       rssi_offset_str(FIELD_GET(E_RSSI_OFFSET_5G_0, val)));
	printf("  5GHz Offset1  : %s\n",
	       rssi_offset_str(FIELD_GET(E_RSSI_OFFSET_5G_1, val)));
	printf("\n");

	printf("[Tx power target]\n");
	val = eep_read_word(mc, E_TEMP_2G_TGT_PWR);
	printf("  2GHz (20MHz)  : %s\n",
	       pwr_target_str(FIELD_GET(E_PWR_2G_TARGET, val)));
	val = eep_read_word(mc, E_PWR_5G_80M_TGT);
	printf("  5GHz (20MHz)  : %s\n",
	       pwr_target_str(FIELD_GET(E_PWR_5G_TARGET, val)));
	printf("\n");

	printf("[Tx power delta]\n");
	val = eep_read_word(mc, E_40M_PWR_DELTA);
	printf("  2GHz 20/40MHz : %s\n",
	       pwr_delta_str(FIELD_GET(E_40M_PWR_DELTA_2G, val)));
	printf("  5GHz 20/40MHz : %s\n",
	       pwr_delta_str(FIELD_GET(E_40M_PWR_DELTA_5G, val)));
	val = eep_read_word(mc, E_PWR_5G_80M_TGT);
	printf("  5GHz 20/80MHz : %s\n",
	       pwr_delta_str(FIELD_GET(E_PWR_5G_80M_DELTA, val)));
	printf("\n");

	printf("[Per channel power table]\n");
	mt7610_dump_channel_power(mc);
	printf("\n");

	printf("[Per rate power table]\n");
	mt7610_dump_rate_power(mc);
	printf("\n");

	printf("[TSSI temperature compensation]\n");
	val = eep_read_word(mc, E_TX_AGC_STEP);
	if (FIELD_GET(E_TX_AGC_STEP_VAL, val) == 0xff)
		printf("  Tx AGC step   : 1.0 dBm (default)\n");
	else
		printf("  Tx AGC step   : %.1f dBm\n",
		       (double)FIELD_GET(E_TX_AGC_STEP_VAL, val) / 2);
	val = eep_read_word(mc, E_TSSI_TCOMP_5G_BOUND);
	printf("  5GHz boundary : %u (channel)\n",
	       FIELD_GET(E_TSSI_TCOMP_5G_BOUND_VAL, val));
	printf("  5GHz group 1  : {%s}\n",
	       mt7610_dump_tssi_tcomp(mc, E_TSSI_TCOMP_5G_1_BASE));
	printf("  5GHz group 2  : {%s}\n",
	       mt7610_dump_tssi_tcomp(mc, E_TSSI_TCOMP_5G_2_BASE));
	printf("\n");

	return 0;
}

CHIP(MT7610, 0x7610, mt7610_eep_parse);
CHIP(MT7612, 0x7612, mt7610_eep_parse);

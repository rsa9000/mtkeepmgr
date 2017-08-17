/**
 * MT7610 EEPROM definitions
 *
 * Copyright (c) 2016, Sergey Ryazanov <ryazanov.s.a@gmail.com>
 */

#ifndef _MT7610_H_
#define _MT7610_H_

#define E_PCI_DEV_ID		0x000a
#define E_PCI_VEN_ID		0x000c

#define E_PCI_SUB_DEV_ID	0x0012
#define E_PCI_SUB_VEN_ID	0x0014

#define E_CMB_AUX_OPT		0x0022	/* Use for CMB_CTRL reg prog */

#define E_XTAL_OPT		0x0024	/* Use for 0x0104 reg prog */

#define E_NIC_CFG0		0x0034
#define E_NIC_CFG0_RX_PATH	0x000f
#define E_NIC_CFG0_RX_PATH_S	0
#define E_NIC_CFG0_TX_PATH	0x00f0
#define E_NIC_CFG0_TX_PATH_S	4
#define E_NIC_CFG0_INT_2G_PA	BIT(8)
#define E_NIC_CFG0_INT_5G_PA	BIT(9)
#define E_NIC_CFG0_EXT_PA_CURR	BIT(10)

#define E_NIC_CFG1		0x0036
#define E_NIC_CFG1_HW_RF_CTRL	BIT(0)	/* Hardware radio control */
#define E_NIC_CFG1_DYN_TX_AGC	BIT(1)	/* Dynamic Tx AGC */
#define E_NIC_CFG1_EXT_2G_LNA	BIT(2)
#define E_NIC_CFG1_EXT_5G_LNA	BIT(3)
#define E_NIC_CFG1_CB_ACCEL_DIS	BIT(4)	/* Dis. cardbus acceleration */
#define E_NIC_CFG1_40M_2G_SB	BIT(5)	/* Sideband 40MHz BW in 2.4 GHz */
#define E_NIC_CFG1_40M_5G_SB	BIT(6)	/* Sideband 40MHz BW in 5 GHz */
#define E_NIC_CFG1_WPS_BUT_EN	BIT(7)	/* Enable WPS button */
#define E_NIC_CFG1_40M_2G_DIS	BIT(8)	/* Dis. 40MHz in 2.4 GHz */
#define E_NIC_CFG1_40M_5G_DIS	BIT(9)	/* Dis. 40MHz in 5 GHz */
#define E_NIC_CFG1_ANT_DIV	BIT(11)	/* Antenna diversity */
#define E_NIC_CFG1_ANT_OPT	BIT(12)
#define E_NIC_CFG1_INT_TX_ALC	BIT(13)	/* Internal Tx ALC */
#define E_NIC_CFG1_COEX		BIT(14)
#define E_NIC_CFG1_DAC_TEST	BIT(15)

#define E_COUNTRY_REGION	0x0038
#define E_COUNTRY_REGION_5G	0x00ff
#define E_COUNTRY_REGION_5G_S	0
#define E_COUNTRY_REGION_2G	0xff00
#define E_COUNTRY_REGION_2G_S	8
#define E_COUNTRY_NONE		0xff
#define E_COUNTRY_CUSTOM	0x1E

#define E_FREQ_OFFSET		0x003a
#define E_FREQ_OFFSET_FO	0x00ff	/* Frequency offset */
#define E_FREQ_OFFSET_FO_S	0

#define E_NIC_CFG2		0x0042
#define E_NIC_CFG2_RX_STREAM	0x000f
#define E_NIC_CFG2_RX_STREAM_S	0
#define E_NIC_CFG2_TX_STREAM	0x00f0
#define E_NIC_CFG2_TX_STREAM_S	4
#define E_NIC_CFG2_COEX_ANT	BIT(8)
#define E_NIC_CFG2_CAL_IN_FLASH	BIT(12)
#define E_NIC_CFG2_XTAL_OPT	0x0600
#define E_NIC_CFG2_XTAL_OPT_S	9
#define E_NIC_CFG2_RXTEMP_C_DIS	BIT(11)	/* Rx temp compensation dis. */

#define E_LNA_GAIN_0		0x0044
#define E_LNA_GAIN_2G		0x00ff	/* dB */
#define E_LNA_GAIN_2G_S		0
#define E_LNA_GAIN_5G_0		0xff00	/* CH36-CH46, dB */
#define E_LNA_GAIN_5G_0_S	8

#define E_RSSI_OFFSET_2G	0x0046
#define E_RSSI_OFFSET_2G_0	0x00ff	/* dB */
#define E_RSSI_OFFSET_2G_0_S	0
#define E_RSSI_OFFSET_2G_1	0xff00	/* dB */
#define E_RSSI_OFFSET_2G_1_S	8

#define E_LNA_GAIN_1		0x0048
#define E_LNA_GAIN_5G_1		0xff00	/* CH100-CH128, dB */
#define E_LNA_GAIN_5G_1_S	8

#define E_RSSI_OFFSET_5G	0x004a
#define E_RSSI_OFFSET_5G_0	0x00ff	/* dB */
#define E_RSSI_OFFSET_5G_0_S	0
#define E_RSSI_OFFSET_5G_1	0xff00	/* dB */
#define E_RSSI_OFFSET_5G_1_S	8

#define E_LNA_GAIN_2		0x004c
#define E_LNA_GAIN_5G_2		0xff00	/* CH132-CH165, dB */
#define E_LNA_GAIN_5G_2_S	8

/* For 40MHz/80MHz power delta unpacking */
#define E_PWR_DELTA_VAL		0x3f	/* 0.5 dBm */
#define E_PWR_DELTA_VAL_S	0
#define E_PWR_DELTA_SIGN	BIT(6)	/* 0 - decrease, 1 - increase */
#define E_PWR_DELTA_EN		BIT(7)

#define E_40M_PWR_DELTA		0x0050	/* 20/40 Tx Power Delta */
#define E_40M_PWR_DELTA_2G	0x00ff	/* See E_PWR_DELTA_xxx */
#define E_40M_PWR_DELTA_2G_S	0
#define E_40M_PWR_DELTA_5G	0xff00	/* See E_PWR_DELTA_xxx */
#define E_40M_PWR_DELTA_5G_S	8

/**
 * Per channel power table consists of several parts:
 * - 2GHz channels (1 - 14)
 * - 5GHz low channels (36 - 64)
 * - 5GHz middle channels (100 - 140)
 * - 5GHz hight channels (149 - 173)
 *
 * Each EEPROM word contains power for two channels. First channel of pair is
 * stored in low part, and second channel stored in hight part of word.
 *
 * Note: 5GHz subbands located in EEPROM back to back, so we could treat them
 * as single big table (in any case channels should be specified by table since
 * there are no analytical expression for them)
 */

#define E_CH_PWR_LO		0x00ff	/* Low part of word */
#define E_CH_PWR_LO_S		0
#define E_CH_PWR_HI		0xff00	/* Hight part of word */
#define E_CH_PWR_HI_S		8

/* Oposite not known documentation, driver treats zero as valid value */
#define E_CH_PWR_MIN		0x01	/* Minimal valid value */
#define E_CH_PWR_MAX		0x3f	/* Maximum valid value */
#define E_CH_PWR_DEFAULT	5	/* Def. value if EEP is out of range */

/**
 * Power table for channels in 2.4GHz band
 * Channels: 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14
 */
#define E_CH_PWR_2G_BASE	0x0052

/**
 * Power table for channels in 5GHz low subband
 * Channels: 36, 38, 40, 44, 46, 48, 52, 54, 56, 60, 62, 64
 */
#define E_CH_PWR_5G_0_BASE	0x0078

/**
 * Power table for channels in 5GHz middle subband
 * Channels: 100, 102, 104, 108, 110, 112, 116, 118, 120, 124, 126, 128, 132, 134, 136, 140
 */
#define E_CH_PWR_5G_1_BASE	0x0084

/**
 * Power table for channels in 5GHz hight subband
 * Channels: 149, 151, 153, 157, 159, 161, 165, 167, 169, 171, 173
 */
#define E_CH_PWR_5G_2_BASE	0x0094

#define E_TEMP_OFFSET		0x00d0
#define E_TEMP_OFFSET_VAL	0xff00	/* Reference temperature (signed) */
#define E_TEMP_OFFSET_VAL_S	8

#define E_80M_PWR_DELTA		0x00d2	/* 20/80 Tx Power Delta */
#define E_80M_PWR_DELTA_5G	0xff00	/* See E_PWR_DELTA_xxx */
#define E_80M_PWR_DELTA_5G_S	8

#define E_5G_SUBBANDS		0x00dc	/* Subbands bounds */
#define E_5G_SUBBANDS_MID_CH	0x00ff	/* Middle channel value */
#define E_5G_SUBBANDS_MID_CH_S	0
#define E_5G_SUBBANDS_HIG_CH	0xff00	/* High channel value */
#define E_5G_SUBBANDS_HIG_CH_S	8

/**
 * Per rate power table consists of several parts:
 * - 2GHz CCK, OFDM, HT/VHT MCS rates
 * - 2/5GHz STBC MCS rates
 * - 5GHz OFDM, HT/VHT MCS rates
 * - 5GHz VHT MCS8,9 rates
 *
 * Each EEPROM word contains values for two rates. Each rate power value
 */
#define E_RATE_PWR_LO		0x00ff
#define E_RATE_PWR_LO_S		0
#define E_RATE_PWR_HI		0xff00
#define E_RATE_PWR_HI_S		8

#define E_RATE_PWR_VAL		0x1f
#define E_RATE_PWR_VAL_S	0
#define E_RATE_PWR_SIGN		0x20

#define E_RATE_PWR_2G_BASE	0x00de
#define E_RATE_PWR_2G_CCK_1_55	(E_RATE_PWR_2G_BASE + 0)
#define E_RATE_PWR_2G_OFDM_6_12	(E_RATE_PWR_2G_BASE + 2)
#define E_RATE_PWR_2G_OFDM_24_48	(E_RATE_PWR_2G_BASE + 4)
#define E_RATE_PWR_2G_MCS_0_2	(E_RATE_PWR_2G_BASE + 6)
#define E_RATE_PWR_2G_MCS_4_6	(E_RATE_PWR_2G_BASE + 8)

#define E_RATE_PWR_STBC_BASE	0x00ec
#define E_RATE_PWR_STBC_MCS_0_2	(E_RATE_PWR_STBC_BASE + 0)
#define E_RATE_PWR_STBC_MCS_4_6	(E_RATE_PWR_STBC_BASE + 2)

#define E_RATE_PWR_5G_BASE	0x0120
#define E_RATE_PWR_5G_OFDM_6_12	(E_RATE_PWR_5G_BASE + 0)
#define E_RATE_PWR_5G_OFDM_24_48	(E_RATE_PWR_5G_BASE + 2)
#define E_RATE_PWR_5G_MCS_0_2	(E_RATE_PWR_5G_BASE + 4)
#define E_RATE_PWR_5G_MCS_4_6	(E_RATE_PWR_5G_BASE + 6)
#define E_RATE_PWR_5G_VHT_8_9	(E_RATE_PWR_5G_BASE + 12)

#define E_TSSI_TCOMP_N		14	/* Number of points */
#define E_TSSI_TCOMP_VAL_MIN	-128
#define E_TSSI_TCOMP_VAL_MAX	127
#define E_TSSI_TCOMP_5G_1_BASE	0x00f0
#define E_TSSI_TCOMP_5G_2_BASE	0x00fe

#define E_TSSI_TCOMP_5G_BOUND	0x010c	/* 5GHz boundary channel */
#define E_TSSI_TCOMP_5G_BOUND_VAL	0x00ff
#define E_TSSI_TCOMP_5G_BOUND_VAL_S	0

#define E_TX_AGC_STEP		0x010e	/* Tx AGC step */
#define E_TX_AGC_STEP_VAL	0x00ff	/* Tx AGC step value, 1/2 dBm */
#define E_TX_AGC_STEP_VAL_S	0

#endif	/* !_MT7610_H_ */

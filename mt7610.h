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
#define E_NIC_CFG0_INT_2G_PA	0x0100
#define E_NIC_CFG0_INT_5G_PA	0x0200
#define E_NIC_CFG0_EXT_PA_CURR	0x0400

#define E_NIC_CFG1		0x0036
#define E_NIC_CFG1_HW_RF_CTRL	0x0001	/* Hardware radio control */
#define E_NIC_CFG1_DYN_TX_AGC	0x0002	/* Dynamic Tx AGC */
#define E_NIC_CFG1_EXT_2G_LNA	0x0004
#define E_NIC_CFG1_EXT_5G_LNA	0x0008
#define E_NIC_CFG1_CB_ACCEL_DIS	0x0010	/* Dis. cardbus acceleration */
#define E_NIC_CFG1_40M_2G_SB	0x0020	/* Sideband 40MHz BW in 2.4 GHz */
#define E_NIC_CFG1_40M_5G_SB	0x0040	/* Sideband 40MHz BW in 5 GHz */
#define E_NIC_CFG1_WPS_BUT_EN	0x0080	/* Enable WPS button */
#define E_NIC_CFG1_40M_2G_DIS	0x0100	/* Dis. 40MHz in 2.4 GHz */
#define E_NIC_CFG1_40M_5G_DIS	0x0200	/* Dis. 40MHz in 5 GHz */
#define E_NIC_CFG1_ANT_DIV	0x0800	/* Antenna diversity */
#define E_NIC_CFG1_ANT_OPT	0x1000
#define E_NIC_CFG1_INT_TX_ALC	0x2000	/* Internal Tx ALC */
#define E_NIC_CFG1_COEX		0x4000
#define E_NIC_CFG1_DAC_TEST	0x8000

#define E_NIC_CFG2		0x0042
#define E_NIC_CFG2_RX_STREAM	0x000f
#define E_NIC_CFG2_RX_STREAM_S	0
#define E_NIC_CFG2_TX_STREAM	0x00f0
#define E_NIC_CFG2_TX_STREAM_S	4
#define E_NIC_CFG2_COEX_ANT	0x0100
#define E_NIC_CFG2_XTAL_OPT	0x0600
#define E_NIC_CFG2_XTAL_OPT_S	9
#define E_NIC_CFG2_RXTEMP_C_DIS	0x0800	/* Rx temp compensation dis. */

#endif	/* !_MT7610_H_ */

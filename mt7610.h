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

#define E_NIC_CFG2		0x0042
#define E_NIC_CFG2_RX_STREAM	0x000f
#define E_NIC_CFG2_RX_STREAM_S	0
#define E_NIC_CFG2_TX_STREAM	0x00f0
#define E_NIC_CFG2_TX_STREAM_S	4
#define E_NIC_CFG2_COEX_ANT	BIT(8)
#define E_NIC_CFG2_XTAL_OPT	0x0600
#define E_NIC_CFG2_XTAL_OPT_S	9
#define E_NIC_CFG2_RXTEMP_C_DIS	BIT(11)	/* Rx temp compensation dis. */

#endif	/* !_MT7610_H_ */

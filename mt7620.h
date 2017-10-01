/**
 * MT7620 EEPROM definitions
 *
 * Copyright (c) 2017, Sergey Ryazanov <ryazanov.s.a@gmail.com>
 */

#ifndef _MT7620_H_
#define _MT7620_H_

#define E_NIC_CFG0		0x0034
#define E_NIC_CFG0_RX_PATH	0x000f
#define E_NIC_CFG0_RX_PATH_S	0
#define E_NIC_CFG0_TX_PATH	0x00f0
#define E_NIC_CFG0_TX_PATH_S	4

#define E_NIC_CFG1		0x0036
#define E_NIC_CFG1_EXT_TX_ALC	BIT(1)	/* External Tx ALC */
#define E_NIC_CFG1_EXT_2G_LNA	BIT(2)
#define E_NIC_CFG1_40M_2G_SB	BIT(5)	/* Sideband 40MHz BW in 2.4 GHz */
#define E_NIC_CFG1_WPS_BUT_EN	BIT(7)	/* Enable WPS button */
#define E_NIC_CFG1_40M_2G_DIS	BIT(8)	/* Dis. 40MHz in 2.4 GHz */
#define E_NIC_CFG1_EXT_LNA	BIT(10)	/* Broadband external LNA */
#define E_NIC_CFG1_INT_TX_ALC	BIT(13)	/* Internal Tx ALC */
#define E_NIC_CFG1_TX0_EXT_PA	BIT(14)
#define E_NIC_CFG1_TX1_EXT_PA	BIT(15)

#define E_NIC_CFG2		0x0042
#define E_NIC_CFG2_RX_STREAM	0x000f
#define E_NIC_CFG2_RX_STREAM_S	0
#define E_NIC_CFG2_TX_STREAM	0x00f0
#define E_NIC_CFG2_TX_STREAM_S	4
#define E_NIC_CFG2_RXTEMP_C_DIS	BIT(11)	/* Rx temp compensation dis. */

#endif	/* !_MT7620_H_ */

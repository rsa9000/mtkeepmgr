/**
 * Main header file
 *
 * Copyright (c) 2016-2020, Sergey Ryazanov <ryazanov.s.a@gmail.com>
 */

#ifndef _MTKEEPMGR_H_
#define _MTKEEPMGR_H_

#include <stdint.h>

#define BIT(__n)			(1 << (__n))

#define FIELD_GET(__field, __val)	(((__val) & __field) >> __field ## _S)

/* Common EEPROM locations */
#define E_CHIPID			0x0000

#define E_VERSION			0x0002
#define E_VERSION_VERSION		0xff00
#define E_VERSION_VERSION_S		8
#define E_VERSION_REVISION		0x00ff
#define E_VERSION_REVISION_S		0

#define E_MACADDR_15_00			0x0004
#define E_MACADDR_31_16			0x0006
#define E_MACADDR_47_32			0x0008

uint16_t eep_read_word(const unsigned offset);

struct chip_desc {
	const char *name;
	uint16_t chipid;
	int (*parse_func)(void);
};

#define CHIP(__name, __chipid, __parse_func)				\
	static struct chip_desc __chip_ ## __name = {			\
		.name = #__name,					\
		.chipid = __chipid,					\
		.parse_func = __parse_func,				\
	};								\
	static struct chip_desc *__chip_ ## __name ## __ptr		\
	__attribute__((used, section(("__chips")))) = 			\
	&__chip_ ## __name

#endif	/* !_MTKEEPMGR_H_ */

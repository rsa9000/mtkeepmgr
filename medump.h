/**
 * Main header file
 *
 * Copyright (c) 2016, Sergey Ryazanov <ryazanov.s.a@gmail.com>
 */

#ifndef _MEDUMP_H_
#define _MEDUMP_H_

#include <stdint.h>

#define FIELD_GET(__field, __val)	(((__val) & __field) >> __field ## _S)

/* Common EEPROM locations */
#define E_CHIPID			0x0000

#define E_VERSION			0x0002
#define E_VERSION_VERSION		0xff00
#define E_VERSION_VERSION_S		8
#define E_VERSION_REVISION		0x00ff
#define E_VERSION_REVISION_S		0

uint16_t eep_read_word(const unsigned offset);

struct chip_desc {
	const char *name;
	uint16_t chipid;
	int (*parse_func)(void);
};

#define CHIP(__name, __chipid, __parse_func)				\
	static struct chip_desc __chip_ ## __name			\
	__attribute__((used)) __attribute__((section("__chips"))) = {	\
		.name = #__name,					\
		.chipid = __chipid,					\
		.parse_func = __parse_func,				\
	}

#endif	/* !_MEDUMP_H_ */

/**
 * Main header file
 *
 * Copyright (c) 2016, Sergey Ryazanov <ryazanov.s.a@gmail.com>
 */

#ifndef _MEDUMP_H_
#define _MEDUMP_H_

#include <stdint.h>

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

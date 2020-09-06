/**
 * Various misc utilities
 *
 * Copyright (c) 2016, Sergey Ryazanov <ryazanov.s.a@gmail.com>
 */

#include <stdio.h>

#include "mtkeepmgr.h"

const char *get_macaddr_str(void)
{
	static char buf[0x20];
	uint16_t val0, val1, val2;

	val0 = eep_read_word(E_MACADDR_15_00);
	val1 = eep_read_word(E_MACADDR_31_16);
	val2 = eep_read_word(E_MACADDR_47_32);

	snprintf(buf, sizeof(buf), "%02x:%02x:%02x:%02x:%02x:%02x",
		 val0 & 0xff, val0 >> 8, val1 & 0xff, val1 >> 8,
		 val2 & 0xff, val2 >> 8);

	return buf;
}

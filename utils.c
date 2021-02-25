/**
 * Various misc utilities
 *
 * Copyright (c) 2016-2021, Sergey Ryazanov <ryazanov.s.a@gmail.com>
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
#include <ctype.h>

#include "mtkeepmgr.h"
#include "utils.h"

const char *get_macaddr_str(struct main_ctx *mc)
{
	static char buf[0x20];
	uint16_t val0, val1, val2;

	val0 = eep_read_word(mc, E_MACADDR_15_00);
	val1 = eep_read_word(mc, E_MACADDR_31_16);
	val2 = eep_read_word(mc, E_MACADDR_47_32);

	snprintf(buf, sizeof(buf), "%02x:%02x:%02x:%02x:%02x:%02x",
		 val0 & 0xff, val0 >> 8, val1 & 0xff, val1 >> 8,
		 val2 & 0xff, val2 >> 8);

	return buf;
}

void hexdump_print(const uint8_t *buf, unsigned int len, unsigned int flags)
{
	const uint8_t *p = buf;
	int i, j;

	for (i = 0; i < len; i += 16) {
		if (flags & HEXDUMP_F_ADDR)
			printf("%08x", i);
		for (j = 0; j < 16; ++j) {
			if (j % 8 == 0)
				printf(" ");
			if (i + j < len)
				printf(" %02x", p[i + j]);
			else
				printf("   ");
		}
		printf(" |");
		for (j = 0; j < 16 && i + j < len; ++j)
			printf("%c", isprint(p[i + j]) ? p[i + j] : '.');
		for (; j < 16; ++j)
			printf(" ");
		printf("|\n");
	}
}

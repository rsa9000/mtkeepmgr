/**
 * Main header file
 *
 * Copyright (c) 2016-2020, Sergey Ryazanov <ryazanov.s.a@gmail.com>
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

#ifndef _MTKEEPMGR_H_
#define _MTKEEPMGR_H_

#include <stdint.h>
#include <stddef.h>

#define ARRAY_SIZE(a)	(sizeof(a)/sizeof(a[0]))

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

struct main_ctx;

uint16_t eep_read_word(struct main_ctx *mc, const unsigned offset);

struct chip_desc {
	const char *name;
	uint16_t chipid;
	int (*parse_func)(struct main_ctx *mc);
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

struct connector_desc {
	const char * const name;
	size_t priv_sz;

	int (*init)(struct main_ctx *mc, const char *arg_str);
	void (*clean)(struct main_ctx *mc);
};

/* Main working context */
struct main_ctx {
	const struct connector_desc *con;	/* Selected connector */
	void *con_priv;				/* Connector state */

	uint8_t eep_buf[0x1000];		/* 4k buffer */
	unsigned eep_len;			/* Actual EERPOM size */
};

#endif	/* !_MTKEEPMGR_H_ */

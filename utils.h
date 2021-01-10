/**
 * Various misc utilities
 *
 * Copyright (c) 2016-2021, Sergey Ryazanov <ryazanov.s.a@gmail.com>
 */

#ifndef _UTILS_H_
#define _UTILS_H_

const char *get_macaddr_str(struct main_ctx *mc);

#define HEXDUMP_F_ADDR		0x0001

void hexdump_print(const uint8_t *buf, unsigned int len, unsigned int flags);

#endif	/* !_UTILS_H_ */

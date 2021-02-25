/*
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
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>

#include <sys/types.h>
#include <sys/stat.h>

#include "mtkeepmgr.h"

struct file_priv {
	int fd;
};

static int file_init(struct main_ctx *mc, const char *arg_str)
{
	struct file_priv *fpd = mc->con_priv;
	struct stat stat;
	int ret, err;

	fpd->fd = open(arg_str, O_RDONLY);
	if (fpd->fd == -1) {
		fprintf(stderr, "filecon: unable to open dump file '%s': %s\n",
			arg_str, strerror(errno));
		goto err;
	}

	if (fstat(fpd->fd, &stat)) {
		fprintf(stderr, "filecon: unable to stat dump file '%s': %s\n",
			arg_str, strerror(errno));
		goto err;
	}

	if (stat.st_size > sizeof(mc->eep_buf)) {
		fprintf(stderr, "filecon: dump file is too big (%lu bytes), analysis will be limited by a %zu bytes\n",
			(unsigned long)stat.st_size, sizeof(mc->eep_buf));
		mc->eep_len = sizeof(mc->eep_buf);
	} else {
		mc->eep_len = stat.st_size;
		if (mc->eep_len % 2 != 0)
			mc->eep_len -= 1;
	}

	ret = read(fpd->fd, mc->eep_buf, mc->eep_len);
	if (ret != mc->eep_len) {
		fprintf(stderr, "filecon: unable to read whole input file: %s\n",
			strerror(errno));
		goto err;
	}

	return 0;

err:
	err = errno;
	if (fpd->fd != -1) {
		close(fpd->fd);
		fpd->fd = -1;
	}

	return -err;
}

static void file_clean(struct main_ctx *mc)
{
	struct file_priv *fpd = mc->con_priv;

	close(fpd->fd);
	fpd->fd = -1;
}

const struct connector_desc con_file = {
	.name = "File",
	.priv_sz = sizeof(struct file_priv),
	.init = file_init,
	.clean = file_clean,
};

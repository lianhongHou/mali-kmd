// SPDX-License-Identifier: GPL-2.0 WITH Linux-syscall-note
/*
 *
 * (C) COPYRIGHT 2014-2024 ARM Limited. All rights reserved.
 *
 * This program is free software and is provided to you under the terms of the
 * GNU General Public License version 2 as published by the Free Software
 * Foundation, and any use by you of this program is subject to the terms
 * of such GNU license.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, you can access it online at
 * http://www.gnu.org/licenses/gpl-2.0.html.
 *
 */

/* Kernel UTF utility functions */

#include <linux/mutex.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/printk.h>

#include <kutf/kutf_utils.h>
#include <kutf/kutf_mem.h>

static char tmp_buffer[KUTF_MAX_DSPRINTF_LEN];

static DEFINE_MUTEX(buffer_lock);

const char *kutf_dvsprintf(struct kutf_mempool *pool, const char *fmt, va_list args)
{
	int len;
	size_t size;
	void *buffer;

	mutex_lock(&buffer_lock);
	len = vsnprintf(tmp_buffer, sizeof(tmp_buffer), fmt, args);

	if (len < 0) {
		pr_err("%s: Bad format dsprintf format %s\n", __func__, fmt);
		goto fail_format;
	}

	if (len >= (int)sizeof(tmp_buffer)) {
		pr_warn("%s: Truncated dsprintf message %s\n", __func__, fmt);
		size = sizeof(tmp_buffer);
	} else {
		size = (size_t)(len + 1);
	}

	buffer = kutf_mempool_alloc(pool, size);
	if (!buffer)
		goto fail_alloc;

	memcpy(buffer, tmp_buffer, size);
	mutex_unlock(&buffer_lock);

	return buffer;

fail_alloc:
fail_format:
	mutex_unlock(&buffer_lock);
	return NULL;
}
EXPORT_SYMBOL(kutf_dvsprintf);

const char *kutf_dsprintf(struct kutf_mempool *pool, const char *fmt, ...)
{
	va_list args;
	const char *string;

	va_start(args, fmt);
	string = kutf_dvsprintf(pool, fmt, args);
	va_end(args);

	return string;
}
EXPORT_SYMBOL(kutf_dsprintf);

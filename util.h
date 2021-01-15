// SPDX-License-Identifier: GPL-2.0+
#ifndef __UTIL_H
#define __UTIL_H

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <sys/time.h>

void msg(const char* format, ... )
{
	struct timeval tv = {};
	va_list arglist;

	gettimeofday(&tv, NULL);

	printf("[%-.8ld.%-.6ld] ",  tv.tv_sec, tv.tv_usec);
	va_start(arglist, format);
	vprintf(format, arglist);
	va_end(arglist);
	fflush(stdout);
}

#endif /* __UTIL_H */

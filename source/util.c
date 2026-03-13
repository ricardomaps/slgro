#include <stdlib.h>

#include <swc.h>

#include "include/util.h"
#include "include/slgro.h"

void die(int ret, const char* fmt, ...)
{
	va_list ap;

	fprintf(stderr, "slgro: ");

	va_start(ap, fmt);
	vfprintf(stderr, fmt, ap);
	va_end(ap);

	fputc('\n', stderr);
	fflush(stderr);

	exit(ret);
}

void _log(FILE* fd, const char* fmt, ...)
{
	va_list ap;

	fprintf(fd, "slgro: ");

	va_start(ap, fmt);
	vfprintf(fd, fmt, ap);
	va_end(ap);

	fputc('\n', fd);
	fflush(fd);
}

void sig_handler(int s)
{
	(void)s;

	if (wm.dpy)
		wl_display_terminate(wm.dpy);
}

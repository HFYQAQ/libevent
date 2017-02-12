#include <stdlib.h>
#include <sys/timeb.h>
#include "evutil.h"

#ifndef HAVE_GETTIMEOFDAY
int evutil_gettimeofday(struct timeval *tv, struct timezone *tz) {
	if (tv == NULL)
		return -1;

	struct timeb tb;

	if (ftime(&tb) < 0)
		return -1;

	tv->tv_sec = tb.time;
	tv->tv_usec = tb.millitm * 1000;

	return 0;
}
#endif

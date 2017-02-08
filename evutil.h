#ifndef _EVUTIL_H_
#define _EVUTIL_H_

#include <sys/time.h>
#include <fcntl.h>
#include "log.h"

#define evutil_timercmp(tvp, uvp, cmp) \
	(((tvp)->tv_sec == (uvp)->tv_sec) ? \
	((tvp)->tv_usec cmp (uvp)->tv_usec) : \
	((tvp)->tv_sec cmp (uvp)->tv_sec))

#define EVUTIL_TIMERADD(tva, tvb, tv)	\
do {	\
	(tv)->tv_sec = (tva)->tv_sec + (tvb)->tv_sec;	\
	(tv)->tv_usec = (tva)->tv_usec + (tvb)->tv_usec;	\
	if ((tv)->tv_usec >= 1000000) {	\
		(tv)->tv_sec++;	\
		(tv)->tv_usec -= 1000000;	\
	}	\
} while(0)

#ifdef HAVE_GETTIMEOFDAY
#define evutil_gettimeofday(tv, tz) gettimeofday((tv), (tz))
#else
int evutil_gettimeofday(struct timeval *, struct timezone *);
#endif

#define EVUTIL_FD_CLOSEONEXEC(fd) \
do {	\
	if (fcntl(fd, F_SETFD, FD_CLOEXEC) < 0)	\
		event_err(1, EVENT_LOG_HEAD "fcntl: ", __FILE__, __FUNCTION__, __LINE__);	\
} while(0)

#define EVUTIL_FD_NONBLOCK(fd)	\
do {	\
	if (fcntl(fd, F_SETFL, O_NONBLOCK) < 0)	\
		event_err(1, EVENT_LOG_HEAD "fcntl: ", __FILE__, __FUNCTION__, __LINE__);	\
} while(0)

#endif

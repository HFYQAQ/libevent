#ifndef _EVUTIL_H_
#define _EVUTIL_H_

#define evutil_timercmp(tvp, uvp, cmp) \
	(((tvp)->tv_sec == (uvp)->tv_sec) ? \
	((tvp)->tv_usec cmp (uvp)->tv_usec) : \
	((tvp)->tv_sec cmp (uvp)->tv_sec))

#endif

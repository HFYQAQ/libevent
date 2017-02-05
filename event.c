#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>
#include "config.h"
#include "event-internal.h"
#include "event.h"
#include "log.h"
#include "evutil.h"
#include "evsignal.h"

static int use_monotonic;

#ifdef HAVE_SELECT
extern struct eventop selectops;
#endif

static struct eventop *eventops[] = {
#ifdef HAVE_SELECT
&selectops,
#endif
NULL
};

static void detect_monotonic();
static int gettime(struct event_base *);

struct event_base *event_base_new() {
	struct event_base *base;
	
	// 为base分配空间	
	if (!(base = (struct event_base *) calloc(1, sizeof(struct event_base))))
		event_err(1, EVENT_LOG_HEAD "calloc: ", __FILE__, __FUNCTION__, __LINE__);

	// 初始化时间量
	detect_monotonic();
	gettime(base);

	// 初始化数据容器
	TAILQ_INIT(&base->eventqueue);
	min_heap_ctor(&base->timeheap);

	// about I/O multiplexing
	for (int i = 0; eventops[i] && !base->evbase; i++) {
		base->evsel = eventops[i];

		base->evbase = base->evsel->init();
	}

	// about Signal(depend on I/O)
	base->sig.socketpair[0] = base->sig.socketpair[1] = -1;

	return 0;
}

void detect_monotonic() {
#if defined HAVE_CLOCK_GETTIME && defined CLOCK_MONOTONIC
	struct timespec ts;

	if (clock_gettime(CLOCK_MONOTONIC, &ts) == 0)
		use_monotonic = 1;
#endif
}

int gettime(struct event_base *base) {
	struct timeval *event_tv = &base->event_tv;

	if (base->tv_cache.tv_sec) {
		// 使用缓存时间
		*event_tv = base->tv_cache;

		return 0;
	}
#if defined HAVE_CLOCK_GETTIME && defined CLOCK_MONOTONIC
	if (use_monotonic) {
		// 使用monotonic时间
		struct timespec ts;

		if (clock_gettime(CLOCK_MONOTONIC, &ts) == -1)
			return -1;
		event_tv->tv_sec = ts.tv_sec;
		event_tv->tv_usec = ts.tv_nsec / 1000;

		return 0;
	}
#endif

	// 使用其它获取时间方式
	return evutil_gettimeofday(event_tv, NULL);
}

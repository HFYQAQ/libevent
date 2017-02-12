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

/* global variables */
struct event_base *current_base = NULL;

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
static int gettime(struct event_base *, struct timeval *);
static void time_correct(struct event_base *, struct timeval *);
static int time_next(struct event_base *, struct timeval **);
static void timeout_process(struct event_base *);
static int event_haveevents(struct event_base *);
static void event_queue_insert(struct event_base *, struct event *, short);
static void event_queue_remove(struct event_base *, struct event *, short);
static void event_base_priority_init(struct event_base *, int);

struct event_base *event_base_new() {
	struct event_base *base;
	
	// 为base分配空间	
	if (!(base = (struct event_base *) calloc(1, sizeof(struct event_base))))
		event_err(1, EVENT_LOG_HEAD "calloc: ", __FILE__, __FUNCTION__, __LINE__);

	// 初始化时间量
	detect_monotonic();
	gettime(base, &base->event_tv);

	// 初始化数据容器
	TAILQ_INIT(&base->eventqueue);
	min_heap_ctor(&base->timeheap);
	event_base_priority_init(base, 1);

	// about I/O multiplexing
	for (int i = 0; eventops[i] && !base->evbase; i++) {
		base->evsel = eventops[i];

		base->evbase = base->evsel->init();
	}
	if (!base->evbase)
		event_errx(1, EVENT_LOG_HEAD "no multiplexing available.", __FILE__, __func__, __LINE__);
	else
		event_log("current multiplexing: %s", base->evsel->name);

	// about Signal(depend on I/O)
	base->sig.socketpair[0] = base->sig.socketpair[1] = -1;

	return base;
}

int event_base_set(struct event_base *base, struct event *ev) {
	if (ev->ev_status != EVLIST_INIT)
		return -1;

	ev->ev_base = base;
	ev->ev_pri = base->nactivequeues / 2;

	return 0;
}

void event_set(struct event *ev, int fd, short type, void *arg, void (*cb)(int, short, void *)) {
	if (!ev) {
		event_warnx("event_set failed, because the argument ev is NULL");
		return;
	}

	ev->ev_base = current_base;

	ev->ev_fd = fd;
	ev->ev_type = type;
	ev->ev_arg = arg;
	ev->ev_cb = cb;

	ev->ev_status = EVLIST_INIT;
	ev->ev_res = 0;

	if (current_base)
		ev->ev_pri = current_base->nactivequeues / 2;

	min_heap_element_init(ev);
}

int event_add(struct event *ev, struct timeval *tv) {
	if (!ev)
		return -1;

	if (tv != NULL && !(ev->ev_status & EVLIST_TIMEOUT)) {
		if (min_heap_reserve(&ev->ev_base->timeheap, min_heap_size(&ev->ev_base->timeheap) + 1) < 0)
			return -1;
	}

	if ((ev->ev_type & (EV_READ | EV_WRITE | EV_SIGNAL)) && !(ev->ev_status & (EVLIST_INSERTED | EVLIST_ACTIVE))) {
		if (!ev->ev_base->evsel->add(ev->ev_base->evbase, ev))
			event_queue_insert(ev->ev_base, ev, EVLIST_INSERTED);
		else
			return -1;
	}

	if (tv != NULL) {
		if (ev->ev_status & EVLIST_TIMEOUT)
			event_queue_remove(ev->ev_base, ev, EVLIST_TIMEOUT);
		if ((ev->ev_status & EVLIST_ACTIVE) && (ev->ev_res & EV_TIMEOUT))
			event_queue_remove(ev->ev_base, ev, EVLIST_ACTIVE);

		struct timeval now;
		gettime(ev->ev_base, &now);
		EVUTIL_TIMERADD(&now, tv, &ev->ev_timeout);

		event_queue_insert(ev->ev_base, ev, EVLIST_TIMEOUT);
	}

	return 0;
}

int event_base_loop(struct event_base *base) {
	int loop = 1;
	struct timeval tv;
	struct timeval *tv_p;

	while(loop) {
		if (!event_haveevents(base)) {
			event_log("current base have no events");

			return 1;
		}

		time_correct(base, &tv);

		tv_p = &tv;
		if (!base->event_active_count)
			time_next(base, &tv_p);
		else
			EVUTIL_TIMERCLEAR(&tv);

		gettime(base, &base->event_tv);
		EVUTIL_TIMERCLEAR(&base->tv_cache);

		if ((base->evsel->process(base, base->evbase, tv_p)) < 0)
			return -1;

		gettime(base, &base->tv_cache);

		timeout_process(base);

		if (base->event_active_count) {
			event_active_process(base);
		}

		printf("\n");
	}

	event_log("loop has been asked to terminnate.");
	return 0;
}

void event_del(struct event *ev) {
	if (!ev->ev_base)
		event_warnx(EVENT_LOG_HEAD "ev(%d) del failed, is not registered", __FILE__, __func__, __LINE__, ev->ev_fd);

	if (ev->ev_status & EVLIST_INSERTED) {
		event_queue_remove(ev->ev_base, ev, EVLIST_INSERTED);
		ev->ev_base->evsel->del(ev->ev_base->evbase, ev);
	}
	if (ev->ev_status & EVLIST_ACTIVE)
		event_queue_remove(ev->ev_base, ev, EVLIST_ACTIVE);
	if (ev->ev_status & EVLIST_TIMEOUT)
		event_queue_remove(ev->ev_base, ev, EVLIST_TIMEOUT);
}

void event_active(struct event *ev, short res) {
	ev->ev_res = res;
	event_queue_insert(ev->ev_base, ev, EVLIST_ACTIVE);
}

void event_active_process(struct event_base *base) {
	event_log("event_active_process");
}

void detect_monotonic() {
#if defined HAVE_CLOCK_GETTIME && defined CLOCK_MONOTONIC
	struct timespec ts;

	if (clock_gettime(CLOCK_MONOTONIC, &ts) == 0)
		use_monotonic = 1;
#endif
}

int gettime(struct event_base *base, struct timeval *tv) {
	if (base->tv_cache.tv_sec) {
		// 使用缓存时间
		*tv = base->tv_cache;

		return 0;
	}
#if defined HAVE_CLOCK_GETTIME && defined CLOCK_MONOTONIC
	if (use_monotonic) {
		// 使用monotonic时间
		struct timespec ts;

		if (clock_gettime(CLOCK_MONOTONIC, &ts) == -1)
			return -1;
		tv->tv_sec = ts.tv_sec;
		tv->tv_usec = ts.tv_nsec / 1000;

		return 0;
	}
#endif

	// 使用其它获取时间方式
	return evutil_gettimeofday(tv, NULL);
}

void time_correct(struct event_base *base, struct timeval *tv) {
	if (use_monotonic)
		return;

	struct timeval off;

	gettime(base, tv);
	if (evutil_timercmp(tv, &base->event_tv, >=)) {
		event_log("time is running normally, need not corrected");
		base->event_tv = *tv;
		return;
	}

	EVUTIL_TIMERSUB(&base->event_tv, tv, &off);

	struct event **p = base->timeheap.p;
	unsigned n = base->timeheap.n;
	for (; n > 0; n--)
		EVUTIL_TIMERSUB(&p[n - 1]->ev_timeout, &off, &p[n - 1]->ev_timeout);
	base->event_tv = *tv;
	event_log("time is running backwards, but now corrected");
}

int time_next(struct event_base *base, struct timeval **tvp_p) {
	struct event *ev;
	struct timeval now;

	if ((ev = min_heap_top(&base->timeheap)) == NULL) {
		*tvp_p = NULL;
		event_log("TIME_NEXT: NULL");

		return 0;
	}

	if (gettime(base, &now) < 0)
		return -1;
	if (evutil_timercmp(&ev->ev_timeout, &now, <=)) {
		EVUTIL_TIMERCLEAR(*tvp_p);
		event_log("TIME_NEXT: 0");

		return 0;
	}

	EVUTIL_TIMERSUB(&ev->ev_timeout, &now, *tvp_p);
	event_log("TIME_NEXT: timeout occur in %ld seconds", (*tvp_p)->tv_sec);

	return 0;
}

void timeout_process(struct event_base *base) {
	struct event *ev;
	struct timeval now;

	if (min_heap_empty(&base->timeheap))
		return;

	gettime(base, &now);

	while ((ev = min_heap_top(&base->timeheap))) {
		if (evutil_timercmp(&ev->ev_timeout, &now, >))
			break;

		event_active(ev, EV_TIMEOUT);
		event_log("TIMEOUT_PROCESS: ev(%d) actived", ev->ev_fd);

		event_del(ev);
	}
}

int event_haveevents(struct event_base *base) {
	return base->event_count > 0;
}

void event_queue_insert(struct event_base *base, struct event *ev, short status) {
	if (ev->ev_status & status) {
		event_errx(1, EVENT_LOG_HEAD "event(%d) already on queue(%d)", __FILE__, __func__, __LINE__, ev->ev_fd, status);
	}

	base->event_count++;
	switch(status) {
	case EVLIST_INSERTED:
		event_log("event insert into INSERTED");
		TAILQ_INSERT_TAIL(&base->eventqueue, ev, ev_next);
		break;
	case EVLIST_ACTIVE:
		event_log("event insert into ACTIVE");
		base->event_active_count++;
		TAILQ_INSERT_TAIL(base->activequeues[ev->ev_pri], ev, ev_active_next);
		break;
	case EVLIST_TIMEOUT:
		event_log("event insert into TIMEOUT");
		min_heap_push(&base->timeheap, ev);
		break;
	default:
		event_errx(1, "unknown queue %d", status);
		break;
	}
	ev->ev_status |= status;
}

void event_queue_remove(struct event_base *base, struct event *ev, short status) {
	if (!(ev->ev_status & status)) {
		event_errx(1, EVENT_LOG_HEAD "event(%d) not on queue(%d)", __FILE__, __func__, __LINE__, ev->ev_fd, status);
	}

	base->event_count--;
	switch(status) {
	case EVLIST_INSERTED:
		event_log("event remove from INSERTED");
		TAILQ_REMOVE(&base->eventqueue, ev, ev_next);
		break;
	case EVLIST_ACTIVE:
		event_log("event remove from ACTIVE");
		base->event_active_count--;
		TAILQ_REMOVE(base->activequeues[ev->ev_pri], ev, ev_active_next);
		break;
	case EVLIST_TIMEOUT:
		event_log("event remove from TIMEOUT");
		min_heap_erase(&base->timeheap, ev);
		break;
	default:
		event_errx(1, "unknown queue %d", status);
		break;
	}
	ev->ev_status &= ~status;
}

void event_base_priority_init(struct event_base *base, int npriority) {
	if (base->event_active_count || base->nactivequeues) {
		event_warnx(EVENT_LOG_HEAD "priority of the base specified has been inited", __FILE__, __func__, __LINE__);
		return;
	}

	base->nactivequeues = npriority;
	if (!(base->activequeues = (struct event_list **) calloc(base->nactivequeues, sizeof(struct event_list *))))
		goto error;
	for (int i = 0; i < base->nactivequeues; i++) {
		if (!(base->activequeues[i] = (struct event_list *) calloc(1, sizeof(struct event_list))))
			goto error;
		TAILQ_INIT(base->activequeues[i]);
	}

	return;

error:
	event_err(1, EVENT_LOG_HEAD "calloc: ", __FILE__, __func__, __LINE__);
}

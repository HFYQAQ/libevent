#ifndef _EVENT_INTERNAL_H_
#define _EVENT_INTERNAL_H_

#include <sys/time.h>
#include "queue.h"
#include "min_heap.h"
#include "evsignal.h"

struct eventop {
	const char *name;
	void *(*init)(void);
};

TAILQ_HEAD(event_list, event);

struct event_base {
	// 时间量
	struct timeval event_tv;
	struct timeval tv_cache;

	// 数据容器
	struct event_list eventqueue;
	min_heap_t timeheap;

	// about Signal
	struct evsignal_info sig;

	// about I/O multiplexing
	struct eventop *evsel;
	void *evbase;
};

#endif

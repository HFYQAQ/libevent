#ifndef _EVENT_H_
#define _EVENT_H_

#include <sys/time.h>
#include "queue.h"

struct event {
	// 存储结构中的位置变量
	TAILQ_ENTRY(event) ev_next;
	unsigned min_heap_idx;

	struct timeval ev_timeout;
};

struct event_base *event_base_new();

#endif

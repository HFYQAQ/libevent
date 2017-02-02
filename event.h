#ifndef _EVENT_H_
#define _EVENT_H_

#include <sys/time.h>
#include "style.h"
#include "queue.h"

#define EVENT_LOG_HEAD FONT_HIGHTLIGHT"%s:%s:%d: "FONT_NONE // <文件名>:<函数名>:<行号>

struct event {
	// 存储结构中的位置变量
	TAILQ_ENTRY(event) ev_next;
	unsigned min_heap_idx;

	struct timeval ev_timeout;
};

struct event_base *event_base_new();

#endif

#ifndef _EVENT_H_
#define _EVENT_H_

#include <sys/time.h>
#include "queue.h"

/* event type */
#define EV_READ 0x01
#define EV_WRITE 0x02
#define EV_SIGNAL 0x04
#define EV_TIMEOUT 0x08

/* event status */
#define EVLIST_INIT 0x01
#define EVLIST_INSERTED 0x02
#define EVLIST_ACTIVE 0x04
#define EVLIST_SIGNAL 0x08
#define EVLIST_TIMEOUT 0x10

struct event_base;
struct event {
	struct event_base *ev_base;

	int ev_fd;
	short ev_type;
	void *ev_arg;
	void (*ev_cb)(int, short, void *);

	short ev_status;
	int ev_pri;

	// 存储结构中的位置变量
	TAILQ_ENTRY(event) ev_next;
	TAILQ_ENTRY(event) ev_active_next;
	unsigned min_heap_idx;

	struct timeval ev_timeout;
};

struct event_base *event_base_new();
void event_set(struct event *, int, short, void *, void (*)(int, short, void *));
int event_add(struct event *, struct timeval *);
#endif

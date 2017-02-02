#include <stdio.h>
#include "queue.h"

struct event {
	TAILQ_ENTRY(event) ev_next;
	int n;
};

struct head;
TAILQ_HEAD(head, event);

int main()
{
	struct head headlist;
	TAILQ_INIT(&headlist);

	struct event event1, event2, event3, event4;
	event1.n = 1;
	event2.n = 2;
	event3.n = 3;
	event4.n = 4;
	TAILQ_INSERT_TAIL(&headlist, &event2, ev_next);
	TAILQ_INSERT_AFTER(&headlist, &event2, &event1, ev_next);
	TAILQ_INSERT_TAIL(&headlist, &event3, ev_next);
	TAILQ_INSERT_TAIL(&headlist, &event4, ev_next);

	struct event *event;
	TAILQ_FOREACH_REVERSE(&headlist, event, ev_next, head)
		printf("%d\n", event->n);
	
	return 0;
}

#include "event-internal.h"
#include "event.h"

struct event_base *event_base_new() {
	struct event_base *base;

	if (!(base = (struct event_base *) calloc(1, sizeof(struct event_base))))
		
}

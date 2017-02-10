#include <sys/time.h>
#include <stdlib.h>
#include "event.h"

int main() {
	struct event_base *base = event_base_new();

	struct event ev1, ev2;
	event_set(&ev1, 0, EV_READ, NULL, NULL);
	event_set(&ev2, 1, EV_WRITE, NULL, NULL);

	event_base_set(base, &ev1);
	event_base_set(base, &ev2);

	struct timeval tv1 = {3, 0};
	struct timeval tv2 = {5, 0};

	event_add(&ev1, &tv1);
//	event_add(&ev2, &tv2);

	event_base_loop(base);

	return 0;
}

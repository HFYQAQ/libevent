#include <sys/time.h>
#include <stdlib.h>
#include "event.h"

int main() {
	struct event_base *base = event_base_new();

	struct event ev1;
	event_set(&ev1, 0, EV_READ, NULL, NULL);

	event_base_set(base, &ev1);

	struct timeval tv;
	tv.tv_sec = 3;
	event_add(&ev1, &tv);

	event_base_loop(base);

	return 0;
}

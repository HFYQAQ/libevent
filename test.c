#include "event.h"

int main() {
	event_base_new();

	struct event ev1;
	event_set(&ev1, 0, EV_READ, NULL, NULL);

	return 0;
}

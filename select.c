#include <stdlib.h>
#include "event-internal.h"
#include "log.h"

void *select_init();

struct eventop selectops = {
	select_init
};

void *select_init() {
	event_log("select_init");
	return NULL;
}

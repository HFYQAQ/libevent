#include <stdio.h>
#include "event.h"
#include "log.h"
#include "log.c"

int main() {
	event_warn(EVENT_LOG_HEAD, __FILE__, __FUNCTION__, __LINE__);
	event_err(1, EVENT_LOG_HEAD, __FILE__, __FUNCTION__, __LINE__);

	return 0;
}

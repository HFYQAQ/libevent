#include <sys/socket.h>
#include "evsignal.h"
#include "event-internal.h"
#include "event.h"
#include "evutil.h"

void evsignal_init(struct event_base *base) {
	if (socketpair(AF_UNIX, SOCK_STREAM, 0, base->sig.socketpair) < 0)
		event_error(1, "socketpair: ");
	EVUTIL_FD_CLOSEONEXEC(base->sig.socketpair[0]);
	EVUTIL_FD_CLOSEONEXEC(base->sig.socketpair[1]);
printf("socketpair okay\n");
}

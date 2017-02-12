#include <sys/socket.h>
#include "evsignal.h"
#include "event-internal.h"
#include "event.h"
#include "evutil.h"
#include "log.h"

void evsignal_init(struct event_base *base) {
	if (socketpair(AF_UNIX, SOCK_STREAM, 0, base->sig.socketpair) < 0)
		event_err(1, EVENT_LOG_HEAD "socketpair: ", __FILE__, __FUNCTION__, __LINE__);
	EVUTIL_FD_CLOSEONEXEC(base->sig.socketpair[0]);
	EVUTIL_FD_CLOSEONEXEC(base->sig.socketpair[1]);
}

#include <sys/socket.h>
#include <signal.h>
#include <string.h>
#include "evsignal.h"
#include "event-internal.h"
#include "event.h"
#include "evutil.h"
#include "log.h"

static struct event_base *signal_base = NULL;

static void evsignal_cb(int, short, void *);
static void evsignal_handler(int);
static void _signo_set_handler(struct event_base *, int, void (*)(int));
static void _signo_restore_handler(struct event_base *, int);

void evsignal_init(struct event_base *base) {
    int i;
    
    signal_base = base;
    
	if (socketpair(AF_UNIX, SOCK_STREAM, 0, base->sig.socketpair) < 0)
		event_err(1, EVENT_LOG_HEAD "socketpair: ", __FILE__, __func__, __LINE__);
	EVUTIL_FD_CLOSEONEXEC(base->sig.socketpair[0]);
	EVUTIL_FD_CLOSEONEXEC(base->sig.socketpair[1]);
    EVUTIL_FD_NONBLOCK(base->sig.socketpair[0]);
    event_log("evsignal_init: socketpair created");
    
    for (i = 0; i < NSIG; i++)
        TAILQ_INIT(&base->sig.sig_evlist[i]);
    memset(base->sig.sig_caught, 0, NSIG * sizeof(sig_atomic_t));
	base->sig.caught = 0;

	base->sig.sa_old_max = 0;
	base->sig.sa_old = NULL;
    
    base->sig.sig_event_added = 0;
    event_set(&base->sig.sig_event, base->sig.socketpair[1], EV_READ | EV_PERSIST, NULL, evsignal_cb);
    base->sig.sig_event.ev_base = base;
	base->sig.sig_event.ev_status |= EVLIST_INTERNAL;
}

void evsignal_add(struct event *ev) {
    if (ev->ev_type != EV_SIGNAL) {
        event_warnx(EVENT_LOG_HEAD "event specified is not signal event", __FILE__, __func__, __LINE__);
        return;
    }

    int signo = ev->ev_fd;
    struct event_base *base = ev->ev_base;
    struct evsignal_info *sig = &base->sig;

    _signo_set_handler(base, signo, evsignal_handler);
    TAILQ_INSERT_TAIL(&sig->sig_evlist[signo], ev, ev_signal_next);

    if (!sig->sig_event_added) {
        event_add(&sig->sig_event, NULL);
        sig->sig_event_added = 1;
    }
}

void _signo_set_handler(struct event_base *base, int signo, void (*handler)(int)) {
    struct sigaction sa;
   
	if (signo >= base->sig.sa_old_max) {
		int n = signo + 1;
		struct sigaction *sa;

		if (!(base->sig.sa_old = realloc(base->sig.sa_old, n * sizeof(struct sigaction *))))
			event_err(1, EVENT_LOG_HEAD "realloc: ", __FILE__, __func__, __LINE__);
		memset(base->sig.sa_old + base->sig.sa_old_max * sizeof(struct sigaction *), 0, (n - base->sig.sa_old_max) * sizeof(struct sigaction *));
		base->sig.sa_old_max = n;
	}
 
    sa.sa_handler = handler;
    sa.sa_flags = SA_RESTART;
    sigfillset(&sa.sa_mask);

	if (!(base->sig.sa_old[signo] = calloc(1, sizeof(struct sigaction))))
		event_err(1, EVENT_LOG_HEAD "calloc: ", __FILE__, __func__, __LINE__);

	if (sigaction(signo, &sa, base->sig.sa_old[signo]) < 0) {
		free(base->sig.sa_old[signo]);
        	event_err(1, EVENT_LOG_HEAD "sigaction: ", __FILE__, __func__, __LINE__);
	}
}

void evsignal_process(struct event_base *base) {
	sig_atomic_t n;
	struct evsignal_info *sig = &base->sig;
	struct event *ev;

	sig->caught = 0;
	for (int i = 0; i < NSIG; i++) {
		if (!sig->sig_caught[i])
			continue;

		n = sig->sig_caught[i];
		sig->sig_caught[i] -= n;
		for (ev = TAILQ_FIRST(&sig->sig_evlist[i]); ev != NULL; ev = TAILQ_NEXT(ev, ev_signal_next)) {
			if (!(ev->ev_type & EV_PERSIST))
				event_del(ev);
			event_active(ev, EV_SIGNAL);
		}
	}
}

void evsignal_del(struct event *ev) {
	TAILQ_REMOVE(&ev->ev_base->sig.sig_evlist[ev->ev_fd], ev, ev_signal_next);
	_signo_restore_handler(ev->ev_base, ev->ev_fd);
}

void _signo_restore_handler(struct event_base *base, int signo) {
	struct sigaction *sa = base->sig.sa_old[signo];
	base->sig.sa_old[signo] = NULL;

	if (sigaction(signo, sa, NULL) < 0) {
		free(sa);
		event_err(1, EVENT_LOG_HEAD "sigaction: ", __FILE__, __func__, __LINE__);
	}
}

void evsignal_destroy(struct event_base *base) {
    
}

void evsignal_cb(int fd, short type, void *arg) {
    char buff;
    
    if (recv(signal_base->sig.socketpair[1], &buff, 1, 0) < 0)
        event_err(1, EVENT_LOG_HEAD "recv: ", __FILE__, __func__, __LINE__);
    
    event_log("evsignal_cb: recv: %c", buff);
}

void evsignal_handler(int signo) {
	signal_base->sig.caught = 1;
    signal_base->sig.sig_caught[signo]++;
   
    if (send(signal_base->sig.socketpair[0], "a", 1, 0) < 0)
        event_err(1, EVENT_LOG_HEAD "send: ", __FILE__, __func__, __LINE__);
    event_log("evsignal_handler: signo(%d) caught", signo);
}

#include <sys/select.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include "event-internal.h"
#include "event.h"
#include "log.h"

#define SELECT_INIT_NEVENTS 8 // event数组初始容量
#define SELECT_MAX_NEVENTS FD_SETSIZE // event数组最大容量

struct selectop {
	int fds;
	int nevents; // event数组容量

	fd_set *readset;
	fd_set *writeset;
	fd_set *readset_dup;
	fd_set *writeset_dup;
	struct event **events_read;
	struct event **events_write;
};

static void *select_init();
static int select_add(void *, struct event *);
static int select_process(struct event_base *, void *, struct timeval *);

static int select_resize(struct selectop *, int);

struct eventop selectops = {
	"select",
	select_init,
	select_add,
	select_process
};

void *select_init() {
	struct selectop *sop;

	if (!(sop = (struct selectop *) calloc(1, sizeof(struct selectop))))
		goto error;

	sop->fds = -1;
	sop->nevents = 0;
	sop->events_read = NULL;
	sop->events_write = NULL;

	if (!(sop->readset = (fd_set *) calloc(1, sizeof(fd_set))))
		goto error;
	if (!(sop->writeset = (fd_set *) calloc(1, sizeof(fd_set))))
		goto error;
	if (!(sop->readset_dup = (fd_set *) calloc(1, sizeof(fd_set))))
		goto error;
	if (!(sop->writeset_dup = (fd_set *) calloc(1, sizeof(fd_set))))
		goto error;
	if (select_resize(sop, SELECT_INIT_NEVENTS) < 0)
		return NULL;

	return sop;

error:
	event_warn(EVENT_LOG_HEAD "calloc: ", __FILE__, __func__, __LINE__);
	return NULL;
}

int select_add(void *arg, struct event *ev) {
	struct selectop *sop = arg;

	if (!ev) {
		event_warnx(EVENT_LOG_HEAD "the arg 2 cannot be NULL", __FILE__, __func__, __LINE__);
		return -1;
	}
	if (sop->fds < ev->ev_fd) {
		// 扩容
		if (sop->nevents < ev->ev_fd + 1) {
			int nevents = (2 * sop->nevents) >= SELECT_MAX_NEVENTS ? SELECT_MAX_NEVENTS : (2 * sop->nevents);
			if (select_resize(sop, nevents) < 0)
				return -1;
		}
		sop->fds = ev->ev_fd;
	}
	if (ev->ev_type & EV_READ) {
		FD_SET(ev->ev_fd, sop->readset);
		sop->events_read[ev->ev_fd] = ev;
	}
	if (ev->ev_type & EV_WRITE) {
		FD_SET(ev->ev_fd, sop->writeset);
		sop->events_write[ev->ev_fd] = ev;
	}

	return 0;
}

int select_process(struct event_base *base, void *arg, struct timeval *tv) {
	if (tv)
		event_log("SELECT_PROCESS: time(%ld:%d) is set", tv->tv_sec, tv->tv_usec);
	else
		event_log("SELECT_PROCESS: tv_sec(NULL) is set");

	struct selectop *sop = arg;
	int i, res;

	memcpy(sop->readset_dup, sop->readset, sizeof(fd_set));
	memcpy(sop->writeset_dup, sop->writeset, sizeof(fd_set));
	res = select(sop->fds + 1, sop->readset_dup, sop->writeset_dup, NULL, tv);
	if (res < 0)
		if (errno != EINTR) {
			event_warn(EVENT_LOG_HEAD "select: ", __FILE__, __func__, __LINE__);
			return -1;
		}

		return 0;
	}
	event_log("select reports %d descripters available", res);

	i = random() % (sop->fds + 1);
	for (int j = 0; j <= sop->fds; j++) {
		res = 0;
		struct event *ev_r = NULL;
		struct event *ev_w = NULL;

		if (++i > sop->fds)
			i = 0;
		if (FD_ISSET(i, sop->readset_dup)) {
			ev_r = sop->events_read[i];
			res |= EV_READ;
		}
		if (FD_ISSET(i, sop->writeset_dup)) {
			ev_w = sop->events_write[i];
			res |= EV_WRITE;
		}

		if (ev_r && (res & ev_r->ev_type))
			event_active(ev_r);
		if (ev_w && (res & ev_w->ev_type))
			event_active(ev_w);
	}

	return 0;
}

int select_resize(struct selectop *sop, int nevents) {
	int nevents_old = sop->nevents;
	struct event **events_read;
	struct event **events_write;

	if (!(events_read = realloc(sop->events_read, nevents * sizeof(struct event *))))
		goto error;
	sop->events_read = events_read;
	if (!(events_write = realloc(sop->events_write, nevents * sizeof(struct event *))))
		goto error;
	sop->events_write = events_write;

	memset(sop->events_read + nevents_old, 0, nevents - nevents_old);
	memset(sop->events_write + nevents_old, 0, nevents - nevents_old);

	sop->nevents = nevents;

	return 0;

error:
	event_warn(EVENT_LOG_HEAD "realloc: ", __FILE__, __func__, __LINE__);
	return -1;
}

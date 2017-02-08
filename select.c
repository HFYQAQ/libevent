#include <sys/select.h>
#include <stdlib.h>
#include <string.h>
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

static int select_resize(struct selectop *, int);

struct eventop selectops = {
	"select",
	select_init,
	select_add
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

#include <sys/select.h>
#include <stdlib.h>
#include <string.h>
#include "event-internal.h"
#include "event.h"
#include "log.h"

#define SELECT_INIT_NEVENTS 8 // event数组初始容量

struct selectop {
	int fds;
	int nevents;

	fd_set *readset;
	fd_set *writeset;
	fd_set *readset_dup;
	fd_set *writeset_dup;
	struct event **events_read;
	struct event **events_write;
};

static void *select_init();

static void select_resize(struct selectop *, int);

struct eventop selectops = {
	"select",
	select_init
};

void *select_init() {
	struct selectop *sop;

	if (!(sop = (struct selectop *) calloc(1, sizeof(struct selectop))))
		goto error;

	sop->fds = 0;
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
	select_resize(sop, SELECT_INIT_NEVENTS);

	return sop;

error:
	event_warn(EVENT_LOG_HEAD "calloc: ", __FILE__, __func__, __LINE__);
	return NULL;
}

void select_resize(struct selectop *sop, int nevents) {
	int nevents_old = sop->nevents;
	struct event **events_read;
	struct event **events_write;

	if (!(events_read = realloc(sop->events_read, nevents * sizeof(struct event *))))
		event_err(1, EVENT_LOG_HEAD "realloc: ", __FILE__, __func__, __LINE__);
	sop->events_read = events_read;
	if (!(events_write = realloc(sop->events_write, nevents * sizeof(struct event *))))
		event_err(1, EVENT_LOG_HEAD "realloc: ", __FILE__, __func__, __LINE__);
	sop->events_write = events_write;

	memset(sop->events_read + nevents_old, 0, nevents - nevents_old);
	memset(sop->events_write + nevents_old, 0, nevents - nevents_old);

	sop->nevents = nevents;
}

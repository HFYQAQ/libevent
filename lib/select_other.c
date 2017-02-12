#include <stdlib.h>
#include <sys/select.h>
#include <string.h>
#include "event-internal.h"
#include "event.h"
#include "log.h"

typedef unsigned long fd_mask;

#define SELECT_INIT_FDSZ 32;

struct selectop {
	int fds; // 最大描述符

	fd_set readset;
	fd_set readset_dup;
	fd_set writeset;
	fd_set writeset_dup;

	struct event **events_read;
	struct event **events_write;
};

void *select_init();

/* 辅助函数 */
#define howmany(x, y) (((x) + (y) - 1) / (y))
static void select_resize(struct selectop *, int);

struct eventop selectops = {
	"select",
	select_init
};

void *select_init() {
	struct selectop *sop;

	if (!(sop = (struct selectop *) calloc(1, sizeof(sop)))) {
		event_warn(EVENT_LOG_HEAD "calloc: ", __FILE__, __FUNCTION__, __LINE__);
		return NULL;
	}

	select_resize(sop, howmany(SELECT_INIT_FDSZ, NFDBITS) * sizeof(fd_mask));

	return NULL;
}

void select_resize(strct selectop *sop, int newsize) {
	int nevents = (newsize / sizeof(fd_mask)) * NFDBITS; // 新event数组容量（个）
	int nevents_old = howmany(sop->fds + 1, NFDBITS) * NFDBITS; // 原event数组容量（个）
	fd_set *readset = NULL;
	fd_set *readset_dup = NULL;
	fd_set *writeset = NULL;
	fd_set *writeset_dup = NULL;
	struct event **events_read = NULL;
	struct event **events_write = NULL;

	if (!(readset = (fd_set *) realloc(sop->readset, newsize)))
		goto error;
	sop->readset = readset;
	if (!(readset_dup = (fd_set *) realloc(sop->readset_dup, newsize)))
		goto error;
	sop->readset_dup = readset_dup;
	if (!(writeset = (fd_set *) realloc(sop->writeset, newsize)))
		goto error;
	sop->writeset = writeset;
	if (!(writeset_dup = (fd_set *) realloc(sop->writeset_dup, newsize)))
		goto error;
	sop->writeset_dup = writeset_dup;
	if (!(events_read = (struct event **) realloc(sop->events_read, nevents * sizeof(struct event *))))
		goto error;
	sop->events_read = events_read;
	if (!(events_write = (struct event **) realloc(sop->events_write, nevents * sizeof(struct event *))))
		goto error;
	sop->events_write = events_write;

	memset(sop->readset + sop->howmany(sop->fds, NFDBITS) * sizeof(fd_mask), 0, newsize - sop->howmany(sop->fds, NFDBITS));

error:
	event_err(1, EVENT_LOG_HEAD "realloc: ", __FILE__, __FUNCTION__, __LINE__);
}

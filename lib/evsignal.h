#ifndef _EVSIGNAL_H_
#define _EVSIGNAL_H_

#include <signal.h>
#include "event_list.h"

struct event_base;
struct event;

struct evsignal_info {
    struct event sig_event;
	int socketpair[2]; // [0]: write    [1]:read
    struct event_list sig_evlist[NSIG];
    sig_atomic_t sig_caught[NSIG];
    
    int sig_event_added;
};

void evsignal_init(struct event_base *);
void evsignal_add(struct event *);
void evsignal_process(struct event_base *);
void evsignal_del(struct event *);
void evsignal_destroy(struct event_base *);

#endif

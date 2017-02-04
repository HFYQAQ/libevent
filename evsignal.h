#ifndef _EVSIGNAL_H_
#define _EVSIGNAL_H_

struct event_base;
struct event;

struct evsignal_info {
	int socketpair[2];
};

void evsignal_init(struct event_base *);
int evsignal_add(struct event *);
int evsignal_del(struct event *);
void evsignal_process(struct event_base *);
void evsignal_destroy(struct event_base *);

#endif

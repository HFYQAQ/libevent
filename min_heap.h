#ifndef _MIN_HEAP_H_
#define _MIN_HEAP_H_

#include <stdlib.h>
#include "event.h"
#include "evutil.h"

typedef struct min_heap {
	struct event **p;
	unsigned n, a;
} min_heap_t;

inline void min_heap_ctor(min_heap_t *);
inline void min_heap_dtor(min_heap_t *);
inline unsigned min_heap_size(min_heap_t *);
inline int min_heap_empty(min_heap_t *);
inline void min_heap_element_init(struct event *);
inline int min_heap_greater(struct event *, struct event *);
inline struct event* min_heap_top(min_heap_t *);
inline int min_heap_reserve(min_heap_t *, unsigned);
inline int min_heap_push(min_heap_t *, struct event *);
inline struct event* min_heap_pop(min_heap_t *);
inline int min_heap_erase(min_heap_t *, struct event *);
inline void min_heap_shift_up(min_heap_t *, unsigned, struct event *);
inline void min_heap_shift_down(min_heap_t *, unsigned, struct event *);

void min_heap_ctor(min_heap_t *s) {
	s->p = 0;
	s->n = 0;
	s->a = 0;
}

void min_heap_dtor(min_heap_t *s) {
	free(s);
}

unsigned min_heap_size(min_heap_t *s) {
	return s->n;
}

int min_heap_empty(min_heap_t *s) {
	return s->n == 0u;
}

void min_heap_element_init(struct event *e) {
	e->min_heap_idx = -1;
}

int min_heap_greater(struct event *a, struct event *b) {
	return evutil_timercmp(&a->ev_timeout, &b->ev_timeout, >);
}

struct event* min_heap_top(min_heap_t *s) {
	return *s->p;
}

int min_heap_reserve(min_heap_t *s, unsigned cap) {
	if (s->a < cap) {
		struct event **p;
		unsigned a = s->a ? s->a * 2 : 8u;

		if (a < cap)
			a = cap;
		if (!(p = (struct event **) realloc(s->p, a * sizeof(struct event *))))
			return -1;
		s->p = p;
		s->a = a;
	}

	return 0;
}

int min_heap_push(min_heap_t *s, struct event *e) {
	if (!min_heap_reserve(s, s->n + 1)) {
		min_heap_shift_up(s, s->n++, e);
		return 0;
	}

	return -1;
}

struct event* min_heap_pop(min_heap_t *s) {
	if (s->n) {
		struct event *e = *s->p;
		min_heap_shift_down(s, 0u, s->p[--s->n]);
		e->min_heap_idx = -1;
		return e;
	}

	return (struct event *) 0;
}

int min_heap_erase(min_heap_t *s, struct event *e) {
	if (e->min_heap_idx == -1u)
		return -1;

	unsigned parent = (e->min_heap_idx - 1) / 2;
	if (e->min_heap_idx > 0 && min_heap_greater(s->p[parent], s->p[--s->n]))
		min_heap_shift_up(s, e->min_heap_idx, s->p[s->n]);
	else
		min_heap_shift_down(s, e->min_heap_idx, s->p[s->n]);
	e->min_heap_idx = -1;

	return 0;
}

void min_heap_shift_up(min_heap_t *s, unsigned hole_idx, struct event *e) {
	unsigned parent = (hole_idx - 1) / 2;
	while (hole_idx && min_heap_greater(s->p[parent], e)) {
		(s->p[hole_idx] = s->p[parent])->min_heap_idx = hole_idx;
		hole_idx = parent;
		parent = (hole_idx - 1) / 2;
	}
	(s->p[hole_idx] = e)->min_heap_idx = hole_idx;
}

void min_heap_shift_down(min_heap_t *s, unsigned hole_idx, struct event *e) {
	unsigned min_child = (hole_idx + 1) * 2;
	while (min_child <= s->n) {
		min_child -= (min_child == s->n || min_heap_greater(s->p[min_child], s->p[min_child - 1]));
		if (min_heap_greater(e, s->p[min_child])) {
			(s->p[hole_idx] = s->p[min_child])->min_heap_idx = hole_idx;
			hole_idx = min_child;
			min_child = (hole_idx + 1) * 2;
		} else
			break;
	}
	min_heap_shift_up(s, hole_idx, e);
}

#endif

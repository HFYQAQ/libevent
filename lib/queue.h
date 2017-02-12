#ifndef _QUEUE_H_
#define _QUEUE_H_

/*
 * Tail Queue declarations.
 */
#define TAILQ_HEAD(head, type)	\
struct head {	\
	struct type *tqh_first;	\
	struct type **tqh_last;	\
}

#define TAILQ_ENTRY(type)	\
struct {	\
	struct type *tqe_next;	\
	struct type **tqe_prev;	\
}

/*
 * Tail Queue access methods.
 */
#define TAILQ_FIRST(head) (head)->tqh_first
#define TAILQ_END(head) NULL
#define TAILQ_NEXT(elm, field) (elm)->field.tqe_next
#define TAILQ_LAST(head, headname) (*(((struct headname *)((head)->tqh_last))->tqh_last))
#define TAILQ_PREV(elm, field, headname) (*(((struct headname *)((elm)->field.tqe_prev))->tqh_last))
#define TAILQ_ISEMPTY(head) (TAILQ_FIRST(head) == TAILQ_END(head))
#define TAILQ_FOREACH(head, elm, field)	\
	for ((elm) = TAILQ_FIRST(head);	\
	     (elm) != TAILQ_END(head);	\
	     (elm) = TAILQ_NEXT((elm), field))

#define TAILQ_FOREACH_REVERSE(head, elm, field, headname)	\
	for ((elm) = TAILQ_LAST(head, headname);	\
	     (elm) != TAILQ_END(head);	\
	     (elm) = TAILQ_PREV(elm, field, headname))

/*
 * Tail Queue functions
 */
#define TAILQ_INIT(head)	\
do {	\
	(head)->tqh_first = NULL;	\
	(head)->tqh_last = &(head)->tqh_first;	\
} while(0)

#define TAILQ_INSERT_HEAD(head, elm, field)	\
do {	\
	if (((elm)->field.tqe_next = TAILQ_FIRST(head)) == TAILQ_END(head))	\
		(head)->tqh_last = &(elm)->field.tqe_next;	\
	else	\
		TAILQ_FIRST(head)->field.tqe_prev = &(elm)->field.tqe_next;	\
	(head)->tqh_first = (elm);	\
	(elm)->field.tqe_prev = &(head)->tqh_first;	\
} while(0)

#define TAILQ_INSERT_TAIL(head, elm, field)	\
do {	\
	(elm)->field.tqe_next = TAILQ_END(head);	\
	(elm)->field.tqe_prev = (head)->tqh_last;	\
	*(head)->tqh_last = (elm);	\
	(head)->tqh_last = &(elm)->field.tqe_next;	\
} while(0)

#define TAILQ_INSERT_AFTER(head, listelm, elm, field)	\
do {	\
	if (((elm)->field.tqe_next = (listelm)->field.tqe_next) == TAILQ_END(head))	\
		(head)->tqh_last = &(elm)->field.tqe_next;	\
	else	\
		(listelm)->field.tqe_next->field.tqe_prev = &(elm)->field.tqe_next;	\
	(listelm)->field.tqe_next = (elm);	\
	(elm)->field.tqe_prev = &(listelm)->field.tqe_next;	\
} while(0)

#define TAILQ_INSERT_BEFORE(listelm, elm, field)	\
do {	\
	(elm)->field.tqe_next = (listelm);	\
	(elm)->field.tqe_prev = (listelm)->field.tqe_prev;	\
	*(listelm)->field.tqe_prev = (elm);	\
	(listelm)->field.tqe_prev = &(elm)->field.tqe_next;	\
} while(0)

#define TAILQ_REMOVE(head, elm, field)	\
do {	\
	if ((elm)->field.tqe_next == TAILQ_END(head))	\
		(head)->tqh_last = (elm)->field.tqe_prev;	\
	else	\
		(elm)->field.tqe_next->field.tqe_prev = (elm)->field.tqe_prev;	\
	*(elm)->field.tqe_prev = (elm)->field.tqe_next;	\
} while(0)

#define TAILQ_REPLACE(head, listelm, elm, field)	\
do {	\
	if (((elm)->field.tqe_next = (listelm)->field.tqe_next) == TAILQ_END(head))	\
		(head)->tqh_last = &(elm)->field.tqe_next;	\
	else	\
		(listelm)->field.tqe_next->field.tqe_prev = &(elm)->field.tqe_next;	\
	(elm)->field.tqe_prev = (listelm)->field.tqe_prev;	\
	*(listelm)->field.tqe_prev = (elm);	\
} while(0)

#endif

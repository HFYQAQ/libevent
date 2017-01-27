#include <stdio.h>
#include <stdlib.h>
#include "min_heap.h"

int main() {
	min_heap_t s;
	struct event e0, e1, e2, e3, e4, e5;

	min_heap_ctor(&s);
	min_heap_element_init(&e0);
	min_heap_element_init(&e1);
	min_heap_element_init(&e2);
	min_heap_element_init(&e3);
	min_heap_element_init(&e4);
	min_heap_element_init(&e5);
	
	e0.ev_timeout.tv_sec = 9;
	e1.ev_timeout.tv_sec = 3;
	e2.ev_timeout.tv_sec = 2;
	e3.ev_timeout.tv_sec = 4;
	e4.ev_timeout.tv_sec = 5;
	e5.ev_timeout.tv_sec = 6;
	
	min_heap_push(&s, &e0);
	min_heap_push(&s, &e1);
	min_heap_push(&s, &e2);
	min_heap_push(&s, &e3);
	min_heap_push(&s, &e4);
	min_heap_push(&s, &e5);
	
	printf("%ld\n", min_heap_pop(&s)->ev_timeout.tv_sec);
	printf("%ld\n", min_heap_pop(&s)->ev_timeout.tv_sec);
	printf("%ld\n", min_heap_pop(&s)->ev_timeout.tv_sec);
	printf("%ld\n", min_heap_pop(&s)->ev_timeout.tv_sec);
	printf("%ld\n", min_heap_pop(&s)->ev_timeout.tv_sec);
	printf("%ld\n", min_heap_pop(&s)->ev_timeout.tv_sec);
	
	min_heap_ctor(&s);

	return 0;
}

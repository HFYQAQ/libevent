test: test.o event.o log.o
	gcc -o test test.o event.o log.o

test.o: test.c event.c event.h event-internal.h queue.h min_heap.h log.c log.h
	gcc -c test.c event.c log.c

clean:
	rm *.o

test: test.o event.o log.o evutil.o evsignal.o select.o
	gcc -o test test.o event.o log.o evutil.o evsignal.o select.o

test.o: test.c
	gcc -c test.c

event.o: event-internal.h event.h event.c
	gcc -c event.c

log.o: log.h log.c
	gcc -c log.c

evutil.o: evutil.h evutil.c
	gcc -c evutil.c

evsignal.o: evsignal.h evsignal.c
	gcc -c evsignal.c

select.o: select.c
	gcc -c select.c

clean:
	rm *.o

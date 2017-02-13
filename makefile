DIR_EV = lib

server: server.o event.o log.o evutil.o evsignal.o select.o
	gcc -o server server.o event.o log.o evutil.o evsignal.o select.o

server.o: server.c
	gcc -c server.c

event.o: $(DIR_EV)/event_list.h $(DIR_EV)/event-internal.h $(DIR_EV)/event.h $(DIR_EV)/event.c
	gcc -c $(DIR_EV)/event.c

log.o: $(DIR_EV)/log.h $(DIR_EV)/log.c
	gcc -c $(DIR_EV)/log.c

evutil.o: $(DIR_EV)/evutil.h $(DIR_EV)/evutil.c
	gcc -c $(DIR_EV)/evutil.c

evsignal.o: $(DIR_EV)/event_list.h $(DIR_EV)/evsignal.h $(DIR_EV)/evsignal.c
	gcc -c $(DIR_EV)/evsignal.c

select.o: $(DIR_EV)/select.c
	gcc -c $(DIR_EV)/select.c

clean:
	rm *.o

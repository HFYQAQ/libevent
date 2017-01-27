test_queue : test_queue.o
	gcc -o test_queue test_queue.o

test_queue.o : test_queue.c queue.h
	gcc -c test_queue.c

clean:
	rm test_queue.o

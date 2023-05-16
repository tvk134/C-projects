CC = gcc
CFLAGS = -g -std=c99 -Wall -fsanitize=address -O

memgrind: mymalloc.o memgrind.o
	$(CC) $(CFLAGS) -o $@ $^

all: memgrind err correctness

err: mymalloc.o err.o
	$(CC) $(CFLAGS) -o $@ $^

correctness: mymalloc.o correctness.o
	$(CC) $(CFLAGS) -o $@ $^

%.o: %.c
	$(CC) $(CFLAGS) -o $@ -c $<

memgrind.o: mymalloc.h
mymalloc.o: mymalloc.h
err.o: mymalloc.h
correctness.o: mymalloc.h

clean:
	rm -f memgrind debug memgrind.o mymalloc-debug.o

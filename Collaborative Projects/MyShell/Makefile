CC = gcc
CFLAGS = -std=c99 -g -Wall -fsanitize=address,undefined


mysh: mysh.o arraylist.o token.o
	$(CC) $(CFLAGS) $^ -o $@

mysh.o arraylist.o token.o: arraylist.h token.h

clean:
	rm -rf *.o demo mysh

all: first

first: truncatingPrime.c
	gcc -g -Wall -Werror -fsanitize=address -std=c11 truncatingPrime.c -o truncatingPrime

clean:
	rm -rf first

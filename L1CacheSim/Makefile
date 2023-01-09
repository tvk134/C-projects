all: first

first: L1CacheSim.c
	gcc -g -Wall -Werror -fsanitize=address -std=c11 -lm L1CacheSim.c -o L1CacheSim

clean:
	rm -rf L1CacheSim

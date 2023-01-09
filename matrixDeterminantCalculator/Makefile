all: first

first: detCalc.c
	gcc -g -Wall -Werror -fsanitize=address -std=c11 detCalc.c -o detCalc

clean:
	rm -rf detCalc


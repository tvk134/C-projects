all: first

first: LLMaintainer.c
	gcc -g -Wall -Werror -fsanitize=address -std=c11 LLMaintainer.c -o LLMaintainer
clean:
	rm -rf LLMaintainer

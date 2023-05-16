#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <strings.h>
#include <fcntl.h>
#include <sys/types.h>

int main(int argc, char **argv){

    char* string = "Hello Tester\n";
    write(STDOUT_FILENO, string, strlen(string));
    return EXIT_SUCCESS;
}
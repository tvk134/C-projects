#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "mymalloc.h"
/*There is an extra case, called case 4 that my TA went over that I'm going to add*/

#define PTR 30
#define OBJSZ 120

int main(int argc, char **argv)
{
    int x, *p;
    char *q;
    // int y; unsigned char *Q[PTR]; // this will be for case 4, there is an extra case they didn't want to give us its accessing a two dimensional array and printing it
    
    int test = argc > 1 ? atoi(argv[1]) : 0;
        
    switch (test) {
    default:
        puts("Missing or invalid test number");
        return EXIT_FAILURE;
    
    case 1:
        free(&x);
        break;
    case 2:
        p = (int *) malloc(sizeof(int) * 10);
        free(p+1);
        break;
    
    case 3:
        p = (int *) malloc(sizeof(int) * 10);
        free(p);
        free(p);
        break;
    
    case 4:
        //p = (int*) malloc(sizeof(int));
        q = (char *)malloc(sizeof(char)*3);
        free(q);
        q = (char*)malloc(4096);
        free(q);
        break;
    }
    
    return EXIT_SUCCESS;
}
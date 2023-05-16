#include <stdio.h>
#include <stdlib.h>
#include "mymalloc.h"

int main(int argc, char const *argv[])
{
    //MEMSIZE is set to 4096. So, allocating 4091 bytes would leave 5 bytes unallocated which should be possible to allocate later.
    //NOTE: THE TOPMOST CHUNK HEADER IS NOT A PART OF THE 4096 MEMSIZE ALLOTED. THE MEMORY HEAP IS INITIALIZED TO SIZE MEMSIZE + 4 TO ACCOUNT FOR THE TOPMOST HEADER. 
    //Of these, only 1 will be usable since 4 are taken up by the chunk header.
    printf("Allocating 4091 bytes...\n");
    int* first = malloc(4091);
    printf("Allocating 1 byte...\n");
    char* second = malloc(1);
    //Now, trying to allocate any more memory will return an error since there should be none left.
    printf("This allocation will return an insufficient memory error...\n");
    char* third = malloc(1); //1 byte is the smallest possible amount we could allocate. It still fails.

    //Freeing the 1 byte we allocated earlier.
    printf("Freeing 1 byte...\n");
    free(second);

    //This allocation will be successful, since the freed and unallocated memory was reserved.
    printf("Allocating last byte again...\n");
    third = malloc(1);
    free(first);
    free(third);
    //It is now clear that the malloc() function reserves unallocated and freed memory.

    printf("\n");

    int* fourth = malloc(sizeof(int)*4);
    int* fifth = malloc(sizeof(int)*4);
    for(int i = 0;i<4;i++){
        fourth[i] = 0;
        fifth[i] = 1;
    }

    //since fourth and fifth are both allocated arrays with values distinct from the other's, printing their contents should show that their contents are the same and do not overlap.
    printf("fourth[0]: %d, fourth[1]: %d, fourth[2]: %d, fourth[3]: %d\n",fourth[0],fourth[1],fourth[2],fourth[3]);
    printf("fifth[0]: %d, fifth[1]: %d, fifth[2]: %d, fifth[3]: %d\n",fifth[0],fifth[1],fifth[2],fifth[3]);
    printf("\n");
    //clearly, they do not overlap since their values are preserved and distinct.
    free(fourth);
    free(fifth);

    int* sixth = malloc(sizeof(int)*10);
    int* seventh = malloc(sizeof(int)*10);
    for(int j = 0;j<10;j++){
        sixth[j] = 666666;
        seventh[j] = 7777777;
    }

    //now we check that the assigned bit pattern has been preserved.
    int preserved = 1;
    for(int k = 0;k<10;k++){
        if(sixth[k]!=666666||seventh[k]!=7777777)
            preserved = 0;
    }

    if(preserved==1)
        printf("All assigned values of sixth and seventh are preserved.\n");
    else printf ("Assigned values were not preserved!\n");

    free(sixth);
    free(seventh);
    printf("\n");

    int* eighth = malloc(4096);
    //If we try to allocate any more memory, we should get an error.
    printf("This allocation will also return an insufficient memory error...\n");
    int* ninth = malloc(5);
    //Now, if we free eighth and try to allocate memory again, it will not return an error since the free function will have functioned as intended.
    free(eighth);
    ninth = malloc(5);
    if(ninth!=NULL)
        printf("Allocation successful after freeing memory!\n");

    //As of now, 4091 of 4096 bytes are unallocated. If we free ninth and try to allocate 4096 bytes, the freed chunk and the unassigned memory will be coalesced.
    free(ninth);
    printf("\n");
    int* tenth = malloc(4096);
    if(tenth!=NULL)
        printf("Allocation of coalesced memory successful!\n");

    printf("\n");

    printf("Now for some error testing...\n");
    int* p;
    p = malloc(5);
    free(tenth);
    p = malloc(5000);
    free(p);
    int* q = malloc(sizeof(int)*12);
    free(q);
    free(q);

    int x = 5;
    free(&x);

    return 0;
}
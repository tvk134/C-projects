#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include "arraylist.h"

#ifndef DEBUG
#define DEBUG 0
#endif

/* Initialize an empty array list with the specified capacity.
 *
 * Returns 1 on success, or 0 if unable to allocate storage
 *
 * capacity must be greater than 0
 */
// We allocate memory for char** (string pointers), but we dont allocate for the string in initialization since we don't know the length of the string before its inserted
int al_init(list_t *list, unsigned capacity){

    if (DEBUG > 1) fprintf(stderr, "Initializing %p with %u\n", list, capacity);
    assert(capacity > 0);

    list->capacity = capacity;
    list->size = 0;
    list->data = malloc(sizeof(char*) * capacity);
    //should return 1, if data succesfully allocated memory
    return (list->data != NULL);
}

//free the whole list
void free_List(list_t *list){
    if (DEBUG > 1) fprintf(stderr, "Destroying %p\n", list);
    for(int i = 0; i < list->size; i++) free(list->data[i]);
    free(list->data);
}

void free_data(list_t *list){
    for(int i = 0; i < list->size; i++) free(list->data[i]);
    list->size = 0;
}

/* Writes the given data to the specified index in the list
 *
 * Returns 1 on success, or 0 if the index is out of range.
 * Does not extend the length of the list.
 */
int al_replace(list_t *list, char* src, int dataSize, int index){
    if (index >= list->size) return 0;
    free(list->data[index]);
    list->data[index] = malloc(sizeof(char)*dataSize +1);
    strcpy(list->data[index], src);
    return 1;
}

/* Appends data to the end of the list
 *
 * Returns 1 on success, or 0 if memory allocation failed.
 */
int al_push(list_t *list, char *src, int dataSize){
    if (list->size == list->capacity) {
    int newcap = list->capacity * 2;
    char **new = realloc(list->data, sizeof(char*) * newcap);

    if (DEBUG) fprintf(stderr, "increase capacity of %p to %d\n", list, newcap);
    if (!new) return 0;
    // NOTE no changes are made to the list until we know allocation succeded 
    list->data = new;
    list->capacity = newcap;
    }

    if (DEBUG > 1) fprintf(stderr, "push %p: %s\n", list, src);

    list->data[list->size] = malloc(sizeof(char)*dataSize +1);
    strcpy( (list->data[list->size]), src);
    ++list->size;
    return 1;
}

// push NULL
int al_pushNULL(list_t *list){
    if (list->size == list->capacity) {
    int newcap = list->capacity * 2;
    char **new = realloc(list->data, sizeof(char*) * newcap);
    if (!new) return 0;
    // NOTE no changes are made to the list until we know allocation succeded 
    list->data = new;
    list->capacity = newcap;
    }
    list->data[list->size] = NULL;
    ++list->size;
    return 1;
}


/* copying last entry to dest, reduces size of list by 1, and remove/frees last entry
 * Returns a 1 on success, or 0 if the list is empty.
 */

int al_pop(char** dest, list_t *list){
    if (list->size == 0) return 0;
    if (DEBUG > 1) fprintf(stderr, "pop:%s at:%p\n",list->data[list->size -1],list->data[list->size -1]);
    strcpy(*dest, list->data[list->size -1]);
    free(list->data[list->size -1]);
    --list->size;

    return 1;
}

void print_list(list_t *list, char* type){
    printf("List Size:%d\n",list->size);
     for(int i = 0; i < list->size; i++){
        if(list->data[i] == NULL) fprintf(stderr,"%s[%d]:NULL\n",type, i);
        else fprintf(stderr,"%s[%d]:%s of size:%ld\n",type, i, list->data[i], strlen(list->data[i]));
    }
}

int search_list(list_t *list, char* find){
     int count = 0;
     for(int i = 0; i < list->size; i++){
        if(strcmp(list->data[i],find) == 0){
            count++;
        }
    }
    return count;
}






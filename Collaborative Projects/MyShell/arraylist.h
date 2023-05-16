#ifndef _ARRAYLIST_H
#define _ARRAYLIST_H

typedef struct {
unsigned size;
unsigned capacity;
//array of stings
char ** data;
} list_t;

/*none = no redirection, redirectIN = redirection input, redirectOUT = redirection output, redirectBoth = I/O pipeIn = pipe input, pipeOut = pipe output
pipe = none, PipeIn, PipeOut*/
typedef enum { none, redirectIN, redirectOUT, redirectBoth, pipeIN, pipeOUT} kind_t;

typedef struct command_t{
    list_t *argv;
    list_t *in;
    list_t *out;
    kind_t type;
    kind_t pipe;
} command_t;

int  al_init(list_t *list, unsigned capacity);
void free_List(list_t *list);
void free_data(list_t *list);
void print_list(list_t *list, char *type);
int search_list(list_t *list, char* find);
int al_replace(list_t *list, char* src, int dataSize, int index);
int al_push(list_t *list, char *src, int dataSize);
int al_pushNULL(list_t *list);
int al_pop(char **dest, list_t *list);

#endif
#ifndef _TOKEN_H
#define _TOKEN_H


int get_token(list_t *tokens, char **Buffer, int linePos);
int setup_command(list_t *tokens);
int exit_mode(list_t *tokens, char **lineBuffer, int fin);

#endif
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <dirent.h>
#include <sys/types.h>
#include <assert.h>
#include "arraylist.h"
#include "token.h"

//list capacity
#ifndef CAPACITY
#define CAPACITY 6
#endif
//DEBUG ON = 1, DEBUG OFF = 0
#ifndef DEBUG
#define DEBUG 0
#endif
// read()'s BUFFER
#ifndef BUFSIZE
#define BUFSIZE 512
#endif

//2 different buffers: a current line buffer, and a read buffer
char *lineBuffer;
int linePos, lineSize;

void append(char *, int);

int main(int argc, char **argv){

    if (argc > 2) {fprintf(stderr,"my shell only takes up to 1 argument\n"); return EXIT_SUCCESS;}
    //setting up list for our tokens
    list_t tokens;
    //list initializer, starting with CAPACITY macro
    al_init(&tokens, CAPACITY);

    //fin = file input, bytes = byte's read, pos = position in file, lstart = line start
    int fin, bytes, pos, lstart;
    char buffer[BUFSIZE];

    // open specified file or read from stdin
    if (argc == 2) {
        fin = open(argv[1], O_RDONLY);
        if (fin == -1) {
            perror(argv[1]);
            exit(EXIT_FAILURE);
        }
    }
    else {
        fin = 0;
    }

    // set up storage for the current line buffer
    lineBuffer = malloc(BUFSIZE);
    lineSize = BUFSIZE;
    linePos = 0;
    int status = 0,tokstatus=0;
    char *mysh1 = "Greetings!\nmysh> ";
    char *mysh2 = "mysh> ";
    char *mysh3 = "!mysh> ";
    char *mysh4 = "mysh: exiting\n";     

    // remind user if they are running in interactive mode
    if (isatty(fin)) {
        write(STDOUT_FILENO,mysh1,strlen(mysh1));
    }
    // read input, each line in a txt file will be written to the lineBuffer
    while ( (bytes = read(fin, buffer, BUFSIZE)) > 0 ) {
        //if (DEBUG) fprintf(stderr, "read %d bytes\n", bytes);
        // search for newlines
        lstart = 0;
        for (pos = 0; pos < bytes; ++pos) {
            if (buffer[pos] == '\n') {
                int thisLen = pos - lstart + 1;
                //if (DEBUG) fprintf(stderr, "finished line %d+%d bytes\n", linePos, thisLen);
                append(buffer + lstart, thisLen);
                tokstatus = get_token(&tokens,&lineBuffer, linePos);
                if(DEBUG){print_list(&tokens,"Token");fprintf(stderr,"TokStatus:%d\n",tokstatus);}
                if(tokstatus == EXIT_FAILURE ) {status = tokstatus; free_data(&tokens);}
                else if(tokens.size == 0) free_data(&tokens);
                //if status is 0, we can setup command 
                else {status = setup_command(&tokens); if(DEBUG)fprintf(stderr,"Status:%d\n",status);}
                if(status == 3) {
                    write(STDOUT_FILENO,mysh4,strlen(mysh4)); 
                    free(lineBuffer);
                    free_List(&tokens);
                    close(fin);
                    exit(EXIT_SUCCESS);
                    }

                linePos = 0;
                lstart = pos + 1;
            }
        }

        if (lstart < bytes) {
            // partial line at the end of the buffer
            int thisLen = pos - lstart;
            //if(DEBUG)fprintf(stderr, "partial line %d+%d bytes\n", linePos, thisLen);
            append(buffer + lstart, thisLen);
        }
        // remind user if they are running in interactive mode
        if (isatty(fin)) {
            if(status==EXIT_FAILURE) write(STDOUT_FILENO,mysh3,strlen(mysh3));   
            else write(STDOUT_FILENO,mysh2,strlen(mysh2));
        }
    }

    if (linePos > 0) {
        // file ended with partial line
        append("\n", 1);
        tokstatus = get_token(&tokens,&lineBuffer, linePos);
        if(DEBUG){print_list(&tokens,"Token");fprintf(stderr,"TokStatus:%d\n",tokstatus);}
        if(tokstatus == EXIT_FAILURE ) {status = tokstatus; free_data(&tokens);}
        else if(tokens.size == 0) free_data(&tokens);
        else{status = setup_command(&tokens); if(DEBUG)fprintf(stderr,"Status:%d\n",status);}
        if(status == 3) {
            write(STDOUT_FILENO,mysh4,strlen(mysh4)); 
            free(lineBuffer);
            free_List(&tokens);
            close(fin);
            exit(EXIT_SUCCESS);
        }
    }

    free(lineBuffer);
    free_List(&tokens);
    close(fin);
    return EXIT_SUCCESS;
    
}

// add specified text to the line buffer, expanding as necessary
// assumes we are adding at least one byte
void append(char *buf, int len)
{
    int newPos = linePos + len;
    if (newPos > lineSize) {
        lineSize *= 2;
        if (DEBUG) fprintf(stderr, "expanding line buffer to %d\n", lineSize);
        assert(lineSize >= newPos);
        lineBuffer = realloc(lineBuffer, lineSize);
        if (lineBuffer == NULL) {
            perror("line buffer");
            exit(EXIT_FAILURE);
        }
    }
    memcpy(lineBuffer + linePos, buf, len);
    linePos = newPos;
}


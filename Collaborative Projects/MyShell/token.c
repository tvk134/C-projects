#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <strings.h>
#include <fcntl.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <assert.h>
#include <glob.h>
#include "arraylist.h"

#ifndef DEBUG
#define DEBUG 0
#endif

static int status = EXIT_SUCCESS;

int get_token(list_t *tokens, char **Buffer, int linePos);
static int wildcard(list_t *tokens);
static int checkcommand(list_t *list, int index);
int setup_command(list_t *tokens);
static int interpret_command(list_t *tokens, list_t *argv_1, list_t *argv_2,list_t *argv_3, command_t *command);
static int interpret_command_p(list_t *tokens, list_t *argv_1, list_t *argv_2, command_t *command);
static int launch_command( command_t *commands);
static int launch_command_p( command_t *commands);
static int launch_command_built( command_t *commands);
static int cd(list_t *tokens);
static int pwd(list_t *tokens);
int exit_mode(list_t *tokens);

int setup_command(list_t *tokens){ 

    if(tokens->size == 0) return status;

    //argv_1 = PipeIn/NoPipe argument list, argv_2 = PipeIn/NoPipe Input/Ouput, argv_3 = PipeOut argument list, argv_ 4 = PipeOut INPUT/OUTPUT
    list_t argv_1, argv_2, argv_3, argv_4, pipeIn, pipeOut;
   
    al_init(&argv_1,1);
    al_init(&argv_2,1);
    al_init(&argv_3,1);
    al_init(&argv_4,1);
    al_init(&pipeIn,1);
    al_init(&pipeOut,1);

    command_t * commands = (command_t*)malloc(sizeof(command_t));
    commands ->argv = NULL; commands ->in = NULL; commands ->out = NULL;
    commands->pipe = none;

    //Case:Pipe - Check if pipe is needed
    int pipeNeeded = search_list(tokens,"|");
    
    //Case: Multiple pipes, return fail(nonesense)
    if(pipeNeeded>1){
        fprintf(stderr,"Syntax Error: This shell implementation only allows for 1 pipe\n");
        (status = EXIT_FAILURE);
    }
    //Case: 1 pipe
    else if(pipeNeeded == 1){

        if(DEBUG)fputs("Piping!\n",stderr);
        //push first token:command to pipeIn list
        al_push(&pipeIn,tokens->data[0],strlen(tokens->data[0]));
        for(int i = 1; i < tokens->size; i++){
            char *token = tokens->data[i];
            if(strcmp(token,"|") == 0){
                //push command to pipeOut list
                for(int j = i+1; j < tokens->size; j++){
                    token = tokens->data[j];
                    al_push(&pipeOut,token,strlen(token));
                }
                break;
            }
            al_push(&pipeIn,token,strlen(token));
        }

        command_t *new = (command_t*)realloc(commands, sizeof(command_t) * 2);
        if (!new) {free(commands); perror("realloc");}
        commands = new; 

        commands[0].pipe = pipeIN;
        status = interpret_command_p(&pipeIn, &argv_1, &argv_2, &(commands[0]));

        //if pipein is correctly formatted proceed
        if(status==0){
            if(DEBUG)print_list(commands[0].argv,"PipeIn");
            if(commands[0].type == redirectIN && DEBUG) print_list(commands[0].in,"Input");

            commands[1].pipe = pipeOUT;
            status = interpret_command_p(&pipeOut, &argv_3, &argv_4, &(commands[1]));
            //if pipeout is correctly formatted proceed
            if(status == 0){
                if(DEBUG)print_list(commands[1].argv,"PipeOut");
                if (commands[1].type == redirectOUT && DEBUG) print_list(commands[1].out,"Output");
                
                if(strcmp(commands[0].argv->data[0],"exit") == 0 || strcmp(commands[0].argv->data[0],"cd") == 0 || strcmp(commands[1].argv->data[0],"exit") == 0 || strcmp(commands[1].argv->data[0],"cd") == 0) launch_command_built(commands);
                else launch_command_p(commands);
            }
        }
    }
    //Case:No pipe
    else if(pipeNeeded == 0){
        if(DEBUG)fputs("No Piping!\n",stderr);
        status = interpret_command(tokens, &argv_1, &argv_2,&argv_3, commands);
        if(status == 0){
            if(DEBUG)print_list(commands->argv,"argv");
            if(commands->type == redirectIN && DEBUG) print_list(commands->in,"Input");
            else if (commands->type == redirectOUT && DEBUG) print_list(commands->out,"Output");
            else if (commands->type == redirectBoth && DEBUG) {print_list(commands->in,"Input");print_list(commands->out,"Output");}
            
            if(strcmp(commands->argv->data[0],"exit") == 0 || strcmp(commands->argv->data[0],"cd") == 0) launch_command_built(commands);
            else launch_command(commands);    
        } 
    }

    //free data from list after successful command and no longer using it
    free_data(tokens);
    free_List(&argv_1);
    free_List(&argv_2);
    free_List(&argv_3);
    free_List(&argv_4);
    free_List(&pipeIn);
    free_List(&pipeOut);
    free(commands);
    //return 0 when command is succesfully executed
    return status;
}

//pipe 
static int interpret_command_p(list_t *tokens, list_t *argv_1, list_t *argv_2, command_t *command){

    for(int i = 0; i < tokens->size; i++){
        
        char *token = tokens->data[i];

        //Start Case: push command into argument list
        if(i == 0){al_push(argv_1,token,strlen(token));}

        //pushing arglist to command(before redirection)
        else if(strcmp(token,"<") != 0 && strcmp(token,">") != 0){
        al_push(argv_1,token,strlen(token));
        }
        //Output file = file that comes after ">" token 
        else if(strcmp(token,">") == 0){
            if(command->pipe == pipeIN){fprintf(stderr,"Syntax Error: can only choose one ouput\n"); return (status = EXIT_FAILURE);} 
            //Output file = argv_2
            command->type = redirectOUT;
            command->argv = argv_1;
            command->out =  argv_2;
            token = tokens->data[i+1];
            al_push(argv_2,token,strlen(token));
            for(int j = i+2; j < tokens->size; j++){
                token = tokens->data[j];
                //Multiple redirections
                if(strcmp(token,">") == 0 || strcmp(token,"<") == 0){fprintf(stderr,"Syntax Error: can only choose one input or ouput\n"); return (status = EXIT_FAILURE);} 
                al_push(argv_1,token,strlen(token));
            }
            //Push NULL terminator to command's argument list for execvp
            al_pushNULL(argv_1);
            return (status = EXIT_SUCCESS);
        }
        //Input file = file that comes after "<" token 
        else if(strcmp(token,"<") == 0){
            if(command->pipe == pipeOUT){fprintf(stderr,"Syntax Error: can only choose one input\n"); return (status = EXIT_FAILURE);} 
            //Input file = argv_2
            command->type = redirectIN;
            command->argv = argv_1;
            command->in =  argv_2;
            token = tokens->data[i+1];
            al_push(argv_2,token,strlen(token));
            for(int j = i+2; j < tokens->size; j++){
                token = tokens->data[j];
                 //Multiple redirections
                if(strcmp(token,">") == 0 || strcmp(token,"<") == 0) {fprintf(stderr,"Syntax Error: can only choose one input or ouput\n"); return (status = EXIT_FAILURE);} 
                al_push(argv_1,token,strlen(token));
            }
            //Push NULL terminator for execvp
            al_pushNULL(argv_1);
            return (status = EXIT_SUCCESS);           
        }
        //Case: no Pipe & No redirection
        if(i == (tokens->size)-1 ){
            command->type = none;
            command->argv = argv_1;
            al_pushNULL(argv_1);
            return (status = EXIT_SUCCESS);
        }
    }

    return (status = EXIT_FAILURE);
}
//no pipe
static int interpret_command(list_t *tokens, list_t *argv_1, list_t *argv_2, list_t *argv_3, command_t *command){

    for(int i = 0; i < tokens->size; i++){
    
        char *token = tokens->data[i];

        //Start Case: push command into argument list
        if(i == 0){al_push(argv_1,token,strlen(token));}
        //pushing arglist to command(before redirection)
        else if(strcmp(token,"<") != 0 && strcmp(token,">") != 0){
        al_push(argv_1,token,strlen(token));
        }
        //Output file = file that comes after ">" token 
        else if(strcmp(token,">") == 0){
            //Output file = argv_2
            command->type = redirectOUT;
            command->argv = argv_1;
            command->out =  argv_2;
            token = tokens->data[i+1];
            al_push(argv_2,token,strlen(token));
            for(int j = i+2; j < tokens->size; j++){
                token = tokens->data[j];
                if(strcmp(token,">") == 0){fprintf(stderr,"Syntax Error: can only choose one output\n"); return (status = EXIT_FAILURE);} 
                //input file = argv_3
                else if (strcmp(tokens->data[j],"<") == 0){
                    command->type = redirectBoth;
                    command->in =  argv_3;
                    token = tokens->data[j+1];
                    al_push(argv_3,token,strlen(token));
                    for(int k = j+2; k < tokens->size; k++){
                        token = tokens->data[k];
                        if(strcmp(token,">") == 0 || strcmp(token,"<") == 0){fprintf(stderr,"Syntax Error: can only choose one input and output\n"); return (status = EXIT_FAILURE);} 
                        else al_push(argv_1,token,strlen(token));
                    }
                    break;
                }
                else al_push(argv_1,token,strlen(token));
            }
            //Push NULL terminator to command's argument list for execvp
            al_pushNULL(argv_1);
            return (status = EXIT_SUCCESS);
        }
        //Input file = file that comes after "<" token 
        else if(strcmp(token,"<") == 0){
            //Input file = argv_2
            command->type = redirectIN;
            command->argv = argv_1;
            command->in =  argv_2;
            token = tokens->data[i+1];
            al_push(argv_2,token,strlen(token));
            for(int j = i+2; j < tokens->size; j++){
                token = tokens->data[j];
                if(strcmp(token,"<") == 0){fprintf(stderr,"Syntax Error: can only choose one input\n"); return (status = EXIT_FAILURE);} 
                //output file = argv_3
                else if (strcmp(token,">") == 0){
                    command->type = redirectBoth;
                    command->out =  argv_3;
                    token = tokens->data[j+1];
                    al_push(argv_3,token,strlen(token));
                    for(int k = j+2; k < tokens->size; k++){
                        token = tokens->data[k];
                        if(strcmp(token,">") == 0 || strcmp(token,"<") == 0){fprintf(stderr,"Syntax Error: can only choose one input and output\n"); return (status = EXIT_FAILURE);} 
                        else al_push(argv_1,token,strlen(token));
                    }
                    break;
                }
                else al_push(argv_1,token,strlen(token));
            }
            //Push NULL terminator for execvp
            al_pushNULL(argv_1);
            return (status = EXIT_SUCCESS);           
        }
        //Case: no Pipe & No redirection
        if(i == (tokens->size)-1 ){
            command->type = none;
            command->argv = argv_1;
            al_pushNULL(argv_1);
            return (status = EXIT_SUCCESS);
        }

    }

    return (status = EXIT_FAILURE);
}
//no pipe
static int launch_command( command_t *commands){
    if(DEBUG)fputs("Launched\n",stderr);

    int fd[2], wstatus, pid, clid;
    pid = fork(); 
    
    //start argument array at index 1
    char ** args = &(commands->argv->data[1]);

    //fork failed, we're parent process, child couldn't be created
    if(pid < 0){perror("fork"); return (status = EXIT_FAILURE);}
    //we're in the child process
    else if(pid == 0){
        if(commands->type == redirectIN){
            if (DEBUG) fprintf(stderr,"Redirection Input Execution\n");
            if ( (fd[0] =  open( commands->in->data[0] , O_RDONLY )) == -1){
                perror(commands->in->data[0]); 
                exit(EXIT_FAILURE);
            }
            dup2(fd[0],STDIN_FILENO);
            close(fd[0]);      
        }
        else if(commands->type == redirectOUT){
            if (DEBUG) fprintf(stderr,"Redirection Output Execution\n");
            if ( (fd[1] =  open(commands->out->data[0] , O_WRONLY|O_CREAT|O_TRUNC, 0640)) == -1){
                perror(commands->out->data[0]); 
                exit(EXIT_FAILURE);
            }
            dup2(fd[1],STDOUT_FILENO);
            close(fd[1]);  
        }
        else if(commands->type == redirectBoth){
            if (DEBUG) fprintf(stderr,"Both Redirection Execution\n");
            if ( (fd[0] =  open( commands->in->data[0] , O_RDONLY )) == -1){
                perror(commands->in->data[0]); 
                exit(EXIT_FAILURE);
            }
            if ( (fd[1] =  open(commands->out->data[0] , O_WRONLY|O_CREAT|O_TRUNC, 0640)) == -1){
                perror(commands->out->data[0]); 
                exit(EXIT_FAILURE);
            }
            dup2(fd[0],STDIN_FILENO);
            dup2(fd[1],STDOUT_FILENO);
            close(fd[0]); 
            close(fd[1]);  
        }

        //Case: check for Built in Command
        int pwdOut = pwd(commands->argv);
        //Built-in Command fails, return 1 error
        if(pwdOut == 1){
            exit(EXIT_FAILURE);
        }
        //Built-in Command, return 0 successfull
        if(pwdOut == 0){
            exit(EXIT_SUCCESS);
        }

        execv(commands->argv->data[0],args);
        perror(commands->argv->data[0]); 
        exit(EXIT_FAILURE);
    }
    //we're in the parent process, waiting on the child process to finish
    else{
        clid = wait(&wstatus);
    }

    return (clid == pid && wstatus == 0 ? (status = EXIT_SUCCESS) : (status = EXIT_FAILURE) );
}
//pipe 
static int launch_command_p( command_t *commands){
    if(DEBUG)fputs("Launched\n",stderr);
    
    int fd[2],p[2], wstatus, pid[2], clid;
    
    if(pipe(p) == -1){perror("pipe"); return (status = EXIT_FAILURE);}

    
    pid[0] = fork(); 

    //start argument array at index 1
    char ** argsIn = &(commands[0].argv->data[1]);
    char ** argsOut = &(commands[1].argv->data[1]);

    //fork failed, we're parent process, child couldn't be created
    if(pid[0] < 0){perror("fork"); return (status = EXIT_FAILURE);}
    //we're in the child input process
    else if(pid[0] == 0){
        close(p[0]);
        //child input process can only have input redirection
        if(commands[0].type == redirectIN){
            if (DEBUG) fprintf(stderr,"P1:Redirection Input Execution\n");
            if ( (fd[0] =  open( commands[0].in->data[0] , O_RDONLY )) == -1){
                perror(commands[0].in->data[0]); 
                exit(status = EXIT_FAILURE);
            }
            dup2(fd[0],STDIN_FILENO);
            close(fd[0]);      
        }

        dup2(p[1],STDOUT_FILENO);
        close(p[1]);

         //Case: check for Built in Command
        int pwdOut = pwd(commands[0].argv);
        //Built-in Command fails, return 1 error
        if(pwdOut == 1){
            exit(EXIT_FAILURE);
        }
        //Built-in Command, return 0 successfull
        if(pwdOut == 0){
            exit(EXIT_SUCCESS);
        }

        execv(commands[0].argv->data[0],argsIn);
        perror(commands[0].argv->data[0]); 
        exit(EXIT_FAILURE);
    }
    pid[1] = fork(); 
    //fork failed, we're parent process, child couldn't be created
    if(pid[1] < 0){perror("fork"); return (status = EXIT_FAILURE);}
    //we're in the child output process
    else if(pid[1] == 0){
        close(p[1]);
        //child input process can only have output redirection
        if(commands[1].type == redirectOUT){
            if (DEBUG) fprintf(stderr,"P2:Redirection Output Execution\n");
            if ( (fd[1] =  open(commands[1].out->data[0] , O_WRONLY|O_CREAT|O_TRUNC, 0640)) == -1){
                perror(commands[1].out->data[0]); 
                exit(EXIT_FAILURE);
            }
            dup2(fd[1],STDOUT_FILENO);
            close(fd[1]);  
        }

        dup2(p[0],STDIN_FILENO);
        close(p[0]);

         //Case: check for Built in Command
        int pwdOut = pwd(commands[1].argv);
        //Built-in Command fails, return 1 error
        if(pwdOut == 1){
            exit(EXIT_FAILURE);
        }
        //Built-in Command, return 0 successfull
        if(pwdOut == 0){
            exit(EXIT_SUCCESS);
        }

        execv(commands[1].argv->data[0],argsOut);
        perror(commands[1].argv->data[0]); 
        exit(EXIT_FAILURE);
    }
    //we're in the parent process, waiting on the child process to finish
    else{
        close(p[0]);
        close(p[1]);
        for(int i = 0; i < 2; i++){
          clid = wait(&wstatus);
          ((clid == pid[0] || clid == pid[1] ) && wstatus == 0) ? (status = EXIT_SUCCESS) : (status = EXIT_FAILURE);
          if(status == EXIT_FAILURE){
            if(i == 0) wait(&wstatus);
            return status;
          }
        }
        
    }
    return status;
}

static int launch_command_built( command_t *commands){

    if(DEBUG)fputs("Launched Built\n",stderr);

    int cdOut = -1, exiT, pwdOut;

    if(commands->pipe == none){
            cdOut = cd(commands->argv);
            exiT = exit_mode(commands->argv);
            pwdOut = pwd(commands->argv);
            if(cdOut == 1 || pwdOut == 1){
                return (status = EXIT_FAILURE);
            }
            else if(cdOut == 0 || pwdOut == 0){
                return (status = EXIT_SUCCESS);
            }
            else if (exiT == 1){
                return (status = 3);
            }
            else return (status = EXIT_FAILURE);
        }

    int p[2];

    if(pipe(p) == -1){perror("pipe"); return (status = EXIT_FAILURE);}

    if(strcmp(commands[0].argv->data[0],"cd") == 0){
        cdOut = 0;
    }
    else if(strcmp(commands[0].argv->data[0],"exit") == 0){
        exiT = exit_mode(commands[0].argv);
        (status = EXIT_SUCCESS);
    }
    else{
        int wstatus, pid, clid;
        pid = fork(); 
        //start argument array at index 1
        char ** args = &(commands[0].argv->data[1]);
        //fork failed, we're parent process, child couldn't be created
        if(pid < 0){perror("fork"); return (status = EXIT_FAILURE);}
        //we're in the child process
        else if(pid == 0){
            if(commands[0].type == redirectIN){
                int fd;
                if (DEBUG) fprintf(stderr,"P1:Redirection Input Execution\n");
                if ( (fd =  open( commands[0].in->data[0] , O_RDONLY )) == -1){
                    perror(commands[0].in->data[0]); 
                    exit(status = EXIT_FAILURE);
                }
                dup2(fd,STDIN_FILENO);
                close(fd);      
            }
            dup2(p[1],STDOUT_FILENO);
            close(p[0]);
            close(p[1]);

            pwdOut = pwd(commands[0].argv);
            if(pwdOut == 1){
                exit(EXIT_FAILURE);
            }
            if(pwdOut == 0){
                exit(EXIT_SUCCESS);
            }

            execv(commands[0].argv->data[0],args);
            perror(commands[0].argv->data[0]); 
            exit(EXIT_FAILURE);
        }
        //we're in the parent process, waiting on the child process to finish
        else{
            clid = wait(&wstatus);
        }
        (clid == pid && wstatus == 0 ? (status = EXIT_SUCCESS) : (status = EXIT_FAILURE));
        if(status == EXIT_FAILURE) return status;
    }


    if(strcmp(commands[1].argv->data[0],"cd") == 0){
        cdOut = 0;
    }
    else if(strcmp(commands[1].argv->data[0],"exit") == 0){
        exiT = exit_mode(commands[1].argv);
        (status = EXIT_SUCCESS);
    }
    else{
        int wstatus, pid, clid;
        pid = fork(); 
        //start argument array at index 1
        char ** args = &(commands[1].argv->data[1]);
        //fork failed, we're parent process, child couldn't be created
        if(pid < 0){perror("fork"); return (status = EXIT_FAILURE);}
        //we're in the child process
        else if(pid == 0){
            if(commands[1].type == redirectOUT){
                int fd;
            if (DEBUG) fprintf(stderr,"P2:Redirection Output Execution\n");
            if ( (fd =  open(commands[1].out->data[0] , O_WRONLY|O_CREAT|O_TRUNC, 0640)) == -1){
                perror(commands[1].out->data[0]); 
                exit(EXIT_FAILURE);
            }
            dup2(fd,STDOUT_FILENO);
            close(fd);  
        }
            dup2(p[0],STDIN_FILENO);
            close(p[0]);
            close(p[1]);
            
            pwdOut = pwd(commands[1].argv);
            if(pwdOut == 1){
                exit(EXIT_FAILURE);
            }
            if(pwdOut == 0){
                exit(EXIT_SUCCESS);
            }

            execv(commands[1].argv->data[0],args);
            perror(commands[1].argv->data[0]); 
            exit(EXIT_FAILURE);
        }
        //we're in the parent process, waiting on the child process to finish
        else{
            clid = wait(&wstatus);
        }
        (clid == pid && wstatus == 0 ? (status = EXIT_SUCCESS) : (status = EXIT_FAILURE));
        if(status == EXIT_FAILURE) return status;
    }
    if (exiT == 1)return (status = 3);
    if(cdOut != -1 ){
        cdOut = cd(commands[cdOut].argv);
        if(cdOut == 1 )return (status = EXIT_FAILURE);
        if(cdOut == 0 ) return (status = EXIT_SUCCESS);
    }
    return status;
}

int exit_mode(list_t *tokens){
    if(strcmp(tokens->data[0],"exit")==0){ 
        return 1;
    }
    return 0;
}
//return 1 = fail, 0 = success, 2 = not cd
static int cd(list_t *tokens){
    if(strcmp(tokens->data[0],"cd")==0){
        
        //cd with no arguments
        if(tokens->size == 2 && tokens->data[1] == NULL){
            char cwd[FILENAME_MAX];
            chdir(getenv("HOME"));
            if (getcwd(cwd, FILENAME_MAX) == NULL) {perror("getcwd() error"); return 1;}
            return 0;
        }
        //more than 1 argument
        else if(tokens->size>3){
            fprintf(stderr,"Error: too many arguments for cd\n");
            return 1;
        }
        // 1 argument
        else if (tokens->size == 3){
            DIR * dir = opendir(tokens->data[1]);
            int stat = chdir(tokens->data[1]);
            if(stat==0 && dir != NULL){
                closedir(dir);
                return 0;
            }
            else{
                //could not find directory or could not open a directory
                perror(tokens->data[1]);
                return 1;
            }
        }
    }
    return 2;
}
//return 1 = fail, 0 = success, 2 = not cd
static int pwd(list_t *tokens){
    if(strcmp(tokens->data[0],"pwd")==0){
        char path[FILENAME_MAX];
        if (getcwd(path, FILENAME_MAX) == NULL) {perror("getcwd() error"); return 1;}

        if(tokens->size>2){
            fprintf(stderr,"Error: too many arguments for pwd\n");
            return 1;
        }
        else if(tokens->size==2 && tokens->data[1] == NULL){
            char dir [strlen(path)+2];
            strcpy(dir,path);
            dir[strlen(path)] = '\n';
            dir[strlen(path)+1] = '\0';
            write(STDOUT_FILENO,dir,strlen(dir));
            return 0;
        }
    }
    return 2;
}

//Works with a file from curr woking dir / absolute pathname /a pathname with "./" or "../" or "~/"
static int wildcard(list_t *tokens){
    for(int i = 0; i < tokens->size; i++){
    glob_t globbuf;
    int length = strlen(tokens->data[i]);
    //check if there is a token with an asterik
        for(int j = 0; j < length; j++){
            if((tokens->data[i][j]) == '*'){
                if( (i-1)>= 0 && (strcmp(tokens->data[i-1],"<") == 0 || strcmp(tokens->data[i-1],">") == 0) ){fprintf(stderr,"Syntax Error: Wildcards are not permmited in redirection targets\n"); return 2;}
                char *wildcard = tokens->data[i];
                //using GLOB_PERIOD flag allows to find hidden files, GLOB_TILDE flag allows to find relative path to HOME DIRECTORY when user uses "~"
                int ret = glob(wildcard, GLOB_NOSORT | GLOB_TILDE_CHECK, NULL, &globbuf);
                //no global found
                char * temp;
                int c = 0;
                if(ret == GLOB_NOMATCH)return 0;
                for(int k = 0; k < globbuf.gl_pathc; k++ ){
                        if(strlen(globbuf.gl_pathv[k]) != 0){
                            temp = globbuf.gl_pathv[k];
                            for(int j = k + 1; j < globbuf.gl_pathc; j++){
                                if(strlen(globbuf.gl_pathv[j]) != 0 && strcasecmp(temp,globbuf.gl_pathv[j]) > 0){
                                    temp = globbuf.gl_pathv[j];
                                }
                            }
                            int glob_len = strlen(temp);
                            if(c == 0) {al_replace(tokens,temp,glob_len,i);c =1;}
                            else al_push(tokens,temp,glob_len);
                            temp[0] = '\0';
                            k = -1;
                        }
                }
                
                globfree(&globbuf);
                return 1;
            }
        }
    }
    return 0;
}

static int home(list_t *tokens){
    for(int i = 0;i<tokens->size;i++){
        if(tokens->data[i][0]=='~'){
            if(strlen(tokens->data[i])>1 && tokens->data[i][1]=='/'){
                char* homeDir = getenv("HOME");
                int homeSize = strlen(homeDir);
                int argSize = strlen(tokens->data[i]);
                if(DEBUG)printf("%s %d %d\n",homeDir,homeSize,argSize);

                char fullDir[homeSize+argSize];
                int argIndex = 1;
                for(int j = 0;j<homeSize+argSize-1;j++){
                    if(j<homeSize)    
                    {
                        fullDir[j] = homeDir[j];
                    }
                    else{
                        fullDir[j] = tokens->data[i][argIndex];
                        argIndex++;
                    }
                }
                fullDir[homeSize+argSize-1] = '\0';
                al_replace(tokens,fullDir,strlen(fullDir),i);
                if(DEBUG)printf("%s\n",fullDir);
                return 1;
            }
            //shell allows "cd ~"
            else{
                char* homeDir = getenv("HOME");
                int homeSize = strlen(homeDir);
                al_replace(tokens,homeDir,homeSize,i);
            }
        }
    }
    
    return 0;
}

//fail = 2, success = 0
static int checkcommand(list_t *list, int index){

    int slash = 0;
    for(int i = 0; i < strlen(list->data[index]); i++){
        if(list->data[index][i] == '/' ){slash = 1; break;}
    }

    //Check if its a built-in, cd or pwd
    if(strcmp(list->data[index],"pwd") == 0 || strcmp(list->data[index],"cd") == 0 || strcmp(list->data[index],"exit") == 0) return 0;
    //Check if commands start with exception
    else if(strcmp(list->data[index],"<") == 0 || strcmp(list->data[index],">") == 0 || strcmp(list->data[index],"|") == 0) {fprintf(stderr,"Syntax Error: expecting command before [%s]\n",list->data[index]);return 2;}
    //Check if command is given with a pathname : command has a "/"

    else if(slash == 1){
        int sIndex;
        for(int i = 0; i < strlen(list->data[index]); i++){
        if(list->data[index][i] == '/' ){sIndex = i;}
        }
        char *name = &(list->data[index][sIndex+1]);
        if(DEBUG) fprintf(stderr,"%s %ld\n",name, strlen(name));

        char *token = (char*)malloc( sizeof(char)*(strlen(name)) + 1);
        memcpy(token,name,strlen(name));
        token[strlen(name)] = '\0';

        al_push(list,token,strlen(token));
        free(token);
        return 0;
    }
    //if not pathname, its a barename 
    else {
        char cwd1[FILENAME_MAX];
        char cwd[FILENAME_MAX];
        if (getcwd(cwd1, FILENAME_MAX) == NULL) {perror("getcwd() error"); return 2;}
        if(chdir("/usr/local/sbin") == 0 && access(list->data[index], F_OK) == 0){
            if(getcwd(cwd, FILENAME_MAX) == NULL) {perror("getcwd() error"); return 2;}
            int toklen = strlen(list->data[index]);
            int pathlen = strlen(cwd);
            char path[pathlen+1+toklen+1];
            strcpy(path,cwd);
            path[pathlen] = '/';
            for(int i = 0; i < toklen; i++) path[pathlen+1+i] = list->data[index][i];
            path[pathlen+1+toklen] = '\0';
            al_push(list,list->data[index],toklen);
            al_replace(list,path,strlen(path),index);
            chdir(cwd1);
            return 0;
        }
        else if(chdir("/usr/local/bin") == 0 && access(list->data[index], F_OK) == 0){
            if(getcwd(cwd, FILENAME_MAX) == NULL) {perror("getcwd() error"); return 2;}
            int toklen = strlen(list->data[index]);
            int pathlen = strlen(cwd);
            char path[pathlen+1+toklen+1];
            strcpy(path,cwd);
            path[pathlen] = '/';
            for(int i = 0; i < toklen; i++) path[pathlen+1+i] = list->data[index][i];
            path[pathlen+1+toklen] = '\0';
            al_push(list,list->data[index],toklen);
            al_replace(list,path,strlen(path),index);
            chdir(cwd1);
            return 0;
        }
        else if(chdir("/usr/sbin") == 0 && access(list->data[index], F_OK) == 0){
            if(getcwd(cwd, FILENAME_MAX) == NULL) {perror("getcwd() error"); return 2;}
            int toklen = strlen(list->data[index]);
            int pathlen = strlen(cwd);
            char path[pathlen+1+toklen+1];
            strcpy(path,cwd);
            path[pathlen] = '/';
            for(int i = 0; i < toklen; i++) path[pathlen+1+i] = list->data[index][i];
            path[pathlen+1+toklen] = '\0';
            al_push(list,list->data[index],toklen);
            al_replace(list,path,strlen(path),index);
            chdir(cwd1);
            return 0;
        }
        else if(chdir("/usr/bin") == 0 && access(list->data[index], F_OK) == 0){
            if(getcwd(cwd, FILENAME_MAX) == NULL) {perror("getcwd() error"); return 2;}
            int toklen = strlen(list->data[index]);
            int pathlen = strlen(cwd);
            char path[pathlen+1+toklen+1];
            strcpy(path,cwd);
            path[pathlen] = '/';
            for(int i = 0; i < toklen; i++) path[pathlen+1+i] = list->data[index][i];
            path[pathlen+1+toklen] = '\0';
            al_push(list,list->data[index],toklen);
            al_replace(list,path,strlen(path),index);
            chdir(cwd1);
            return 0;
        }
        else if(chdir("/sbin") == 0 && access(list->data[index], F_OK) == 0){
            if(getcwd(cwd, FILENAME_MAX) == NULL) {perror("getcwd() error"); return 2;}
            int toklen = strlen(list->data[index]);
            int pathlen = strlen(cwd);
            char path[pathlen+1+toklen+1];
            strcpy(path,cwd);
            path[pathlen] = '/';
            for(int i = 0; i < toklen; i++) path[pathlen+1+i] = list->data[index][i];
            path[pathlen+1+toklen] = '\0';
            al_push(list,list->data[index],toklen);
            al_replace(list,path,strlen(path),index);
            chdir(cwd1);
            return 0;
        }
        else if(chdir("/bin") == 0 && access(list->data[index], F_OK) == 0){
            if(getcwd(cwd, FILENAME_MAX) == NULL) {perror("getcwd() error"); return 2;}
            int toklen = strlen(list->data[index]);
            int pathlen = strlen(cwd);
            char path[pathlen+1+toklen+1];
            strcpy(path,cwd);
            path[pathlen] = '/';
            for(int i = 0; i < toklen; i++) path[pathlen+1+i] = list->data[index][i];
            path[pathlen+1+toklen] = '\0';
            al_push(list,list->data[index],toklen);
            al_replace(list,path,strlen(path),index);
            chdir(cwd1);
            return 0;
        }
        chdir(cwd1);
        perror(list->data[index]);
        return 2;
    }
    return 2;
}

static int exceptionCheck(list_t *list){
    for(int i = 0; i < list->size; i++){
        if(strcmp(list->data[i],"<") == 0){
            if( (i+1)<list->size && (strcmp(list->data[i+1],"<") == 0 || strcmp(list->data[i+1],">") == 0 || strcmp(list->data[i+1],"|") == 0) ){
                fprintf(stderr,"Syntax Error: expecting file after [<]\n");
                return 1;
            }
        }
        else if(strcmp(list->data[i],">") == 0){
            if( (i+1)<list->size && (strcmp(list->data[i+1],"<") == 0 || strcmp(list->data[i+1],">") == 0 || strcmp(list->data[i+1],"|") == 0) ){
                fprintf(stderr,"Syntax Error: expecting file after [>]\n");
                return 1;
            }
        }
        else if(strcmp(list->data[i],"|") == 0){
            if( (i+1)<list->size && ( strcmp(list->data[i+1],"<") == 0 || strcmp(list->data[i+1],">") == 0 || strcmp(list->data[i+1],"|") == 0) ){
                fprintf(stderr,"Syntax Error: expecting argument after [|]\n");
                return 1;
            }
        }
    }
    return 0;
}

//Error = 1, success = 0, Empty = 2
int get_token(list_t *tokens, char **Buffer, int linePos){
    char *lineBuffer = *Buffer;
    //count = lengthgth of token, buff_pos = position of buffer, start_token = position of where token starts in the buffer
    int count = 0, buff_pos = 0, start_token = 0, pipeIndex = -1;
    assert(lineBuffer[linePos-1] == '\n');
    // search through line buffer for tokens
    while (buff_pos < linePos - 1) {
        // ignore leading whitespaces
        if(lineBuffer[buff_pos] == ' ' && (buff_pos) == 0) {
            ++buff_pos;
            while(lineBuffer[buff_pos] == ' '){
                //Nonesense: line is just whitespaces 
                if(buff_pos == linePos - 2){return (status);}
                ++buff_pos;
            }
            //since we exit loop successfully we hit a non-whitespace character
            start_token = buff_pos;
            count = 0;
            //jumps to top
            continue;
        }
        else{
            if(exceptionCheck(tokens) == 1) return (status = EXIT_FAILURE);
            //Case Exceptions: "<, >, |" (We hit a pipe or redirection)
            char c = lineBuffer[buff_pos];
            if( c == '<' || c == '>' || c =='|' ){
                //Check if there's a token before the exception without whitespaces
                if((buff_pos-1) >= 0 && ( (lineBuffer[buff_pos-1] != ' ') && (lineBuffer[buff_pos-1] != '<') && (lineBuffer[buff_pos-1] != '>') && (lineBuffer[buff_pos-1] != '|') ) ){
                    //write token before the exception into a char array
                    char tok[count + 1];
                    for(int i = 0; i < count; i++){
                        tok[i] = lineBuffer[i + start_token];
                    }

                    tok[count] ='\0';
                    //push the token to the list
                    al_push(tokens,tok, count);
                    if(wildcard(tokens) == 2)return (status = EXIT_FAILURE);
                    home(tokens);
                    //Command Checker: Checks first token pushed | Checks command after pipe
                    if(tokens->size == 1 && checkcommand(tokens,0) == 2) return (status = EXIT_FAILURE);
                    if( pipeIndex != -1 && pipeIndex+1 < tokens->size){
                        if(checkcommand(tokens,pipeIndex+1) == 2) return (status = EXIT_FAILURE);
                        pipeIndex = -1;
                    }
                }

                //write exception token into a char array
                char tok[2];
                tok[0] = c;
                tok[1] ='\0';
                if(c == '|') pipeIndex = tokens->size;
                //push the token to the list
                al_push(tokens,tok, 1);
                //Command Checker: Checks first token pushed | Checks command after pipe
                    if(tokens->size == 1 && checkcommand(tokens,0) == 2) return (status = EXIT_FAILURE);
                    if( pipeIndex != -1 && pipeIndex+1 < tokens->size){
                        if(checkcommand(tokens,pipeIndex+1) == 2) return (status = EXIT_FAILURE);
                        pipeIndex = -1;
                }
                //Syntax Checker: redirection before pipe
                if ( pipeIndex >= 1 && (strcmp(tokens->data[pipeIndex-1],"<") == 0 || strcmp(tokens->data[pipeIndex-1],">") == 0) ) {fprintf(stderr,"Syntax Error: expecting file after [%s]\n",tokens->data[pipeIndex-1]); return (status = EXIT_FAILURE);}

                //Case Whitespace: Trailing whitespaces & Multiple whitespaces after a token & (finds the start of a token if it catches a non-whitespace)
                ++buff_pos;
                while(lineBuffer[buff_pos] == ' '){
                    //Syntax Checker: rest of the line has whitespaces, this means it ends with a pipe or redirection
                    if(buff_pos == linePos - 2){fprintf(stderr,"Syntax Error: expecting file after [%c]\n",c);return (status = EXIT_FAILURE);}
                    ++buff_pos;
                }
                //since we exit loop successfully we hit a non-whitespace character
                start_token = buff_pos;
                count = 0;

                //Syntax Checker: Command/subcommands ends with a pipe or redirection
                if (linePos-1 == buff_pos) {fprintf(stderr,"Syntax Error: expecting file after [%c]\n",c);return (status = EXIT_FAILURE);}
                //jumps to top
                continue;
            }

            //Case 1: We hit a whitespace that is followed by a non-whitespace 
            if((buff_pos-1) >= 0 && lineBuffer[buff_pos] == ' ' && lineBuffer[buff_pos-1] != ' ' ){
                //write our token into a char array
                char tok[count + 1];
                for(int i = 0; i < count; i++){
                    tok[i] = lineBuffer[i + start_token];
                }
                tok[count] ='\0';
                //push the token to the list
                al_push(tokens,tok, count);
                if(wildcard(tokens) == 2)return (status = EXIT_FAILURE);
                home(tokens);
                if(tokens->size == 1 && checkcommand(tokens,0) == 2) return (status = EXIT_FAILURE);
                if( pipeIndex != -1 && pipeIndex+1 < tokens->size){
                    if(checkcommand(tokens,pipeIndex+1) == 2) return (status = EXIT_FAILURE);
                    pipeIndex = -1;
                }
                //Case Whitespace: Trailing whitespaces & Multiple whitespaces after a token & (finds the start of a token if it catches a non-whitespace)
                ++buff_pos;
                while(lineBuffer[buff_pos] == ' '){
                    //rest of the line has whitespaces, just retur
                    if(buff_pos == linePos - 2){return (status = EXIT_SUCCESS);}
                    ++buff_pos;
                }
                //since we exit loop successfully we hit a non-whitespace character
                start_token = buff_pos;
                count = 0;
                //jumps to top
                continue;
            }
            //Case 2: last token at the end of the line doesn't end with a whitespace
            else if(buff_pos == (linePos - 2) && lineBuffer[buff_pos] != ' ' ){
                count++;
                char tok[count + 1];
                for(int i = 0; i < count; i++){
                    tok[i] = lineBuffer[i + start_token];
                }
                tok[count] ='\0';
                //push the token to the list
                al_push(tokens,tok, count);
                if(wildcard(tokens) == 2)return (status = EXIT_FAILURE);
                home(tokens);
                if(tokens->size == 1 && checkcommand(tokens,0) == 2) return (status = EXIT_FAILURE);
                if( pipeIndex != -1 && pipeIndex+1 < tokens->size){
                    if(checkcommand(tokens,pipeIndex+1) == 2) return (status = EXIT_FAILURE);
                    pipeIndex = -1;
                }
                //break the loop since we are at the end of the line
                return (status = EXIT_SUCCESS);
            }
             //Case: line = "\n"
            else if(buff_pos == (linePos - 2) && tokens->size == 0) return (status);

            //Case 3: Iterating though a token, incrementing buffer position and token size
            ++count;
            ++buff_pos;
        }
       
    }
    return (status = EXIT_SUCCESS);
}
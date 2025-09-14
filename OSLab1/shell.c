#include <stdio.h>
#include <stdlib.h>
#include <readline/readline.h>
#include <readline/history.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>

#include "forknexecute.h"

#define MAX_TOKENS 1024

volatile pid_t foreground_pid = 0; //current process id
int shell_terminal_fd; //parent terminal id 

// set up a flag to make sure & and | are mutually exclusive

//quit current foreground process ctrl-c
void sigint_handler(int signum) {
    if (foreground_pid != 0) {
       
        kill(foreground_pid, SIGINT);
    } 
}

// stop the foreground process ctrl-z
void sigtstp_handler(int signum) {
    if (foreground_pid != 0) {

        kill(foreground_pid, SIGTSTP);
    }
}

int valid_command(char **command){
    char *path_env = getenv("PATH");
        if (path_env == NULL) {
            return 1;
        }
        char *path_copy = strdup(path_env);
        char *dir = strtok(path_copy, ":");
        while (dir != NULL) {
            char full_path[MAX_TOKENS];
            
            snprintf(full_path, sizeof(full_path), "%s/%s", dir, command[0]);
            
            if(access(full_path, X_OK) == 0){
                free(path_copy);
                return 0;
            }
            dir = strtok(NULL, ":");
        }
        free(path_copy);
        return 1;
}


int main(void){
    char *input;
    int pipe = 0;
    int background = 0;

    shell_terminal_fd = STDIN_FILENO; //0

    signal(SIGTSTP, sigtstp_handler);
    signal(SIGINT, sigint_handler);
    signal(SIGTTIN, SIG_IGN);
    signal(SIGTTOU, SIG_IGN);

    signal(SIGINT, sigint_handler);
    while (1) {
        //read user input
        input = readline("# ");

        if(!input){
            printf("\n");
            break;
        }
        if(input){

            char *input_copy = strdup(input);

            char *args1[MAX_TOKENS];
            char *args2[MAX_TOKENS];

            char *saveptr = NULL;
            int argv1 = 0;
            int argv2 = 0;

            int fail = 0;
            int pipe_check = 0;

            char *tok = strtok_r(input_copy, " ", &saveptr);
            while(tok) {
                if(strcmp(tok, ">") == 0 || strcmp(tok, "<") == 0 || strcmp(tok, "2>") == 0){
                    args1[argv1++] = tok;
                    tok = strtok_r(NULL, " ", &saveptr);
                    args1[argv1++] = tok;
                    tok = strtok_r(NULL, " ", &saveptr);
    
                }
                else if(strcmp(tok, "|") == 0){
                    pipe_check++;
                    tok = strtok_r(NULL, " ", &saveptr);
                }
                else if(pipe_check == 1){
                    args2[argv2++] = tok;
                    tok = strtok_r(NULL, " ", &saveptr);
                }
                else if (pipe_check > 1){
                    fail = 1;
                    break;
                }
                else{
                    args1[argv1++] = tok;
                    tok = strtok_r(NULL, " ", &saveptr);
                }
            }
            args1[argv1] = NULL; 
            args2[argv1] = NULL;
            int fail1 = 0;
            int fail2 = 0;

            if(pipe_check == 1){
                fail1 = valid_command(args1);
                fail2 = valid_command(args2);
                if (fail1 == 0 && fail2 == 0){
                    fail = 1;
                }
                else{
                    fail = 1;
                }
            }
            else{
                if(strcmp(args1[argv1-1], "&") == 0){
                    background = 1;
                    args1[argv1-1] = NULL;
                    argv1--;
                }
                fail = valid_command(args1);
            }
        
            if (fail == 0){
                if(pipe_check == 1){
                    int process_return = pipe_execute(args1, args2, argv1, argv2, background, &foreground_pid, shell_terminal_fd);
                }
                else if (pipe_check == 0){
                    int process_return = forknexecute(args1, argv1, background, &foreground_pid, shell_terminal_fd);
                }
            }
           
            free(input);
            free(input_copy);
        }
    }

    return 0;
}
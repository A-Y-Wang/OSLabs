#include <stdio.h>
#include <stdlib.h>
#include <readline/readline.h>
#include <readline/history.h>
#include <unistd.h>
#include <string.h>

#include "forknexecute.h"

#define MAX_TOKENS 1024

volatile pid_t foreground_pid = 0;
int shell_terminal_fd; // File descriptor for the terminal

// --- Signal Handler for SIGINT (Ctrl-C) ---
void sigint_handler(int signum) {
    if (foreground_pid != 0) {
        // If there is a foreground process, send SIGINT to it
        kill(foreground_pid, SIGINT);
    }
    // If there is no foreground process, do nothing (shell ignores Ctrl-C)
}

// --- Signal Handler for SIGTSTP (Ctrl-Z) ---
void sigtstp_handler(int signum) {
    if (foreground_pid != 0) {
        // If there is a foreground process, send SIGTSTP to it
        kill(foreground_pid, SIGTSTP);
    }
    // If there is no foreground process, do nothing (shell ignores Ctrl-Z)
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

    while (1) {
        //read user input
        input = readline("# ");

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
                if (fail1 == 1 || fail2 == 1){
                    fail = 1;
                }
            }
            else{
                fail = valid_command(args1);
            }
        
            if (fail == 0){
                if(pipe_check == 1){
                    int process_return = pipe_execute(args1, args2, argv1, argv2);
                }
                else if (pipe_check == 0){
                    int process_return = forknexecute(args1, argv1);
                }
            }
           
            free(input);
            free(input_copy);
        }
    }

    return 0;
}
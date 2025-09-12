#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <string.h>

#define MAX_TOKENS 128

void parser(char **args, int argv, char **clean_args){

    char *input_file = NULL;
    char *output_file = NULL;
    char *error_file = NULL;

    int fd_in;
    int fd_out;
    int fd_err;
    
    int args_counter = 0;

    for(int i = 0; i < argv; i++){
            
        if(strcmp(args[i], "<") == 0){
            if(args[i+1] != NULL){
            input_file = args[i+1];

            fd_in = open(input_file, O_RDONLY);
            if(fd_in < 0 || dup2(fd_in, STDIN_FILENO) < 0){
                exit(EXIT_FAILURE);
            }
            close(fd_in);
            i++;
            }
        }
        else if(strcmp(args[i], ">") == 0){
            if(args[i+1] != NULL){
                output_file = args[i+1];

                fd_out = open(output_file, O_WRONLY | O_CREAT | O_TRUNC, 0644);
                if (fd_out < 0 || dup2(fd_out, STDOUT_FILENO) < 0){
                    exit(EXIT_FAILURE);
                }
                close(fd_out);
                i++;
            }
        }
        else if(strcmp(args[i], "2>") == 0){
            if(args[i+1] != NULL){
                error_file = args[i+1];

                fd_err = open(error_file, O_WRONLY | O_CREAT | O_TRUNC, 0644);
                if (fd_err < 0 || dup2(fd_err, STDERR_FILENO)<0){
                    exit(EXIT_FAILURE);
                }
                close(fd_err);
                i++;
            }
        }
        else {
            clean_args[args_counter] = args[i];
            args_counter++;
        }
    }

    clean_args[args_counter] = NULL;

}
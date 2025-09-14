#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <string.h>

#include "parser.h"

#define MAX_TOKENS 128

int forknexecute(char **args, int argv, int background, volatile pid_t *foreground_pid, int shell_terminal_fd){

    //clean the commands with max 1 pipe
    char *clean_args[MAX_TOKENS];

    pid_t pid = fork();

    if (pid < 0) {
        perror("fork failed");
        return 1;

    } else if (pid == 0) {
        // make cases based on the command??
        setpgid(0, 0);

        signal(SIGINT, SIG_DFL);
        signal(SIGTSTP, SIG_DFL);

        parser(args, argv, clean_args);

        // execvp only returns on error, so we must exit.
        if (execvp(clean_args[0], clean_args) == -1) {
            exit(EXIT_FAILURE);
        }
        
    } else {
        // we are the parent and need to wait for the child to finish? (but only if not a job control command??)
        setpgid(pid, pid);
        if(!background){
            *foreground_pid = pid;
            tcsetpgrp(shell_terminal_fd, pid);
            waitpid(pid, NULL, WUNTRACED);
            tcsetpgrp(shell_terminal_fd, getpgrp());
            *foreground_pid = 0;
        }
        else{
            printf("Started background process with PID %d\n", pid);
        }

    }

    return 0; 

}

int pipe_execute(char **args1, char **args2, int argv1, int argv2, int background, volatile pid_t *foreground_pid, int shell_terminal_fd){

    char *clean_args1[MAX_TOKENS];
    char *clean_args2[MAX_TOKENS];

    int pipefd[2];
    if (pipe(pipefd) == -1) {
        perror("pipe failed");
        return 1;
    }

    pid_t pid1 = fork();

    if (pid1 < 0) {
        perror("fork failed");
        return 1;
    } else if (pid1 == 0) {

        setpgid(0, 0);
        signal(SIGINT, SIG_DFL);
        signal(SIGTSTP, SIG_DFL);
    
        close(pipefd[0]); // Close unused read end
        dup2(pipefd[1], STDOUT_FILENO); // Redirect stdout to pipe write end
        close(pipefd[1]); // Close original write end

        parser(args1, argv1, clean_args1);

        if (execvp(clean_args1[0], clean_args1) == -1) {
            exit(EXIT_FAILURE);
        }
    }

    pid_t pid2 = fork();

    if (pid2 < 0) {
        perror("fork failed");
        return 1;
    } else if (pid2 == 0) {

        setpgid(0, pid1);
        signal(SIGINT, SIG_DFL);
        signal(SIGTSTP, SIG_DFL);

    
        close(pipefd[1]); // Close unused write end
        dup2(pipefd[0], STDIN_FILENO); // Redirect stdin to pipe read end
        close(pipefd[0]); // Close original read end

        parser(args2, argv2, clean_args2);

        if (execvp(clean_args2[0], clean_args2) == -1) {
            exit(EXIT_FAILURE);
        }
    }

    // Parent process

    setpgid(pid1, pid1);
    setpgid(pid2, pid1);

    *foreground_pid = pid1;
    tcsetpgrp(shell_terminal_fd, pid1);

    close(pipefd[0]);
    close(pipefd[1]);

    waitpid(pid1, NULL, WUNTRACED);
    waitpid(pid2, NULL, WUNTRACED);

    tcsetpgrp(shell_terminal_fd, getpgrp());

    *foreground_pid = 0;

    return 0;
}
#ifndef FORKNEXECUTOR_H
#define FORKNEXECUTOR_H

int forknexecute(char **args, int argv, char *command, int background, volatile pid_t *foreground_pid, int shell_terminal_fd);

int pipe_execute(char **args1, char **args2, int argv1, int argv2, int background, volatile pid_t *foreground_pid, int shell_terminal_fd);

#endif 
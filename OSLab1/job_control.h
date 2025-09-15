#ifndef JOB_CONTROL_H
#define JOB_CONTROL_H

#include <sys/types.h>
#include "job_control.h"

#define MAX_JOBS 16
#define MAX_TOKENS 2048


// 'extern' tells other files that this global variable exists,
// but it is DEFINED in job_control.c.

typedef struct{
    pid_t pid;
    int job_id;
    int background; //& or not 1 for background
    char* status; //RUNNING or STOPPED
    char* display_status; //most recent or not
    char* command;
}Job;
// These are the functions that other files (like shell.c) can call.
extern volatile pid_t foreground_pid;
extern int shell_terminal_fd;
extern Job job_list[MAX_JOBS];

void init_job_list(Job* jobs);
int find_max_job_id(Job* jobs);
void add_job(pid_t pid, int status, char* command, int background);
void set_and_clear_done_jobs(Job* job_list);
void print_jobs(Job* jobs);

void fg_command();
void bg_command();

#endif // JOB_CONTROL_H

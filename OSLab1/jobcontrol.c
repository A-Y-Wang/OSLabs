#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>

#include "job_control.h"

#define MAX_TOKENS 2048
#define MAX_JOBS 16

Job job_list[MAX_JOBS];

int find_max_job_id(Job* job_list){
    int max = 0;
    for(int i = 0; i < MAX_JOBS; i++){
        if(job_list[i].job_id > max){
            max = job_list[i].job_id;
        }
    }
    return max;
}

void init_job_list(Job* jobs){
    for(int i = 0; i < MAX_JOBS; i++){
        job_list[i].pid = 0;
        job_list[i].job_id = 0;
        job_list[i].background = 0;
        int status = -1;
        job_list[i].display_status = NULL;
        job_list[i].command = NULL;
    }
}

void add_job(pid_t pid, int status, char* command, int background){
    int added_job = 0;
    for(int i = 0; i < MAX_JOBS; i++){
        if(job_list[i].pid == 0 && added_job == 0){
            job_list[i].pid = pid;
            job_list[i].job_id = find_max_job_id(job_list) + 1;
            job_list[i].background = background;
        
            job_list[i].command = strdup(command);

            if(status == 1){
                job_list[i].status = "Running";
            }
            else{
                job_list[i].status = "Stopped";
            }
            job_list[i].display_status = "+"; //added job is the most current job

            added_job = 1;

        }
        else if(job_list[i].pid != 0){
            // update other job details
            job_list[i].display_status = "-";
            return;
        }
    }
        
}

//only display background job when done
void set_and_clear_done_jobs(Job* job_list) {
    int status;
    pid_t pid;

    while ((pid = waitpid(-1, &status, WNOHANG)) > 0) {
        //if enter it means our os has reported jobs have finished
        for (int i = 0; i < MAX_JOBS; i++) {
            if (job_list[i].pid == pid) {

                printf("\n[%d]%s  Done    %s", job_list[i].job_id, job_list[i].display_status, job_list[i].command);
                
                // Remove the job from our list by setting its PID back to 0.
                job_list[i].pid = 0; 
                free(job_list[i].command);
                break; 
            }
        }
    }
}

void print_jobs(Job* job_list) {
    for (int i = 0; i < MAX_JOBS; i++) {
        if (job_list[i].pid != 0) {
            printf("[%d]%s  %s    %s\n", job_list[i].job_id, job_list[i].display_status, job_list[i].status, job_list[i].command);
        }
    }
}

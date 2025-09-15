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

int find_max_job_id(Job* job_list){
    int max = 0;
    for(int i = 0; i < MAX_JOBS; i++){
        if(job_list[i].job_id > max){
            max = job_list[i].job_id;
        }
    }
    return max;
}

int find_max_index(Job* job_list){ //should also be the current job with +
    int max_index = 0;
    int max = 0;
    for(int i = 0; i < MAX_JOBS; i++){
        if(job_list[i].job_id > max){
            max = job_list[i].job_id;
            max_index = i;
        }
    }
    return max_index; 
}

int find_max_stopped_job(Job* job_list){
    int max = 0;
    int index = 0;
    for(int i = 0; i < MAX_JOBS; i++){
        if(job_list[i].job_id > max && strcmp(job_list[i].status, "Stopped") == 0){
            max = job_list[i].job_id;
        }
    }
    return index;
}

//must hit ctrl z before using fg

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
        }
    }
    return; //done with looping over the job list     
}

//only display background job when done
void set_and_clear_done_jobs(Job* job_list) {
    int status;
    pid_t pid;
    int index = 0;

    while ((pid = waitpid(-1, &status, WNOHANG)) > 0) {
        //if enter it means our os has reported jobs have finished
        for (int i = 0; i < MAX_JOBS; i++) {
            if (job_list[i].pid == pid) {

                printf("\n[%d]%s  Done    %s\n", job_list[i].job_id, job_list[i].display_status, job_list[i].command);
                
                // remove job, set pid = 0, command is freed, job_id=0
                job_list[i].pid = 0; 
                free(job_list[i].command);
                job_list[i].job_id = 0;
                break; 
            }
        }
        index = find_max_index(job_list);
        job_list[index].display_status = "+"; //set the most recent job
    }
}

void print_jobs(Job* job_list) {
    for (int i = 0; i < MAX_JOBS; i++) {
        if (job_list[i].pid != 0) {
            printf("[%d]%s  %s    %s\n", job_list[i].job_id, job_list[i].display_status, job_list[i].status, job_list[i].command);
        }
    }
}

void fg_command() {
    int index = find_max_index(job_list); //finds the index of the most recent job
    if(job_list[index].pid == 0){
        return; //no jobs in the list yet
    }
    //continue this pid job
    if (kill(job_list[index].pid, SIGCONT) < 0) {
        perror("kill (SIGCONT) failed");
        return;
    }

    job_list[index].status = "Running";
    job_list[index].display_status = "+"; //since brought to the foreground, it is the most recent job
    if (job_list[index].background == 1){
        int len = strlen(job_list[index].command);
        job_list[index].command[len - 1] = '\0'; //remove the & from the command string
        job_list[index].background = 0; //change to foreground job
    }

    printf("%s\n", job_list[index].command); //print the updated command

    tcsetpgrp(shell_terminal_fd, job_list[index].pid); //give the terminal to the job
    foreground_pid = job_list[index].pid; //set the foreground pid to this job's pid
    int status;
    waitpid(job_list[index].pid, &status, WUNTRACED);
    tcsetpgrp(shell_terminal_fd, getpgrp());
    foreground_pid = 0;

    if (WIFSTOPPED(status)) {
        job_list[index].status = "Stopped"; 
    } else {
        job_list[index].pid = 0;
        job_list[index].job_id = 0; //foreground job finished, remove it from the list.
        int index_to_change = find_max_index(job_list);
        job_list[index_to_change].display_status = "+";
    }
    return;
}

void bg_command(){
    int index = find_max_stopped_job(job_list);
    if(job_list[index].pid == 0){
        return;
    }
    //continue the stopped job in the background
    if(kill(job_list[index].pid, SIGCONT) < 0){
        perror("kill (SIGCONT) failed");
        return;
    }

    char temp_command[MAX_TOKENS];
    
    // strncpy(temp_command, job->command, sizeof(temp_command) - 1);
    // temp_command[sizeof(temp_command) - 1] = '\0';
    // int len = strlen(temp_command);
    // while (len > 0 && (isspace((unsigned char)temp_command[len - 1]) || temp_command[len - 1] == '&')) {
    //     temp_command[--len] = '\0';
    // }
    // printf("%s\n", temp_command);
    return;
}

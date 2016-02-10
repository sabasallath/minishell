#ifndef __JOBS_H__
#define __JOBS_H__

#include "csapp.h"
#include <string.h>

#define MAXJOBS 12

typedef enum {
	FREE = 0,
	STOPPED,
	RUNNING,
	DONE,
} JobStatus;

typedef struct {
	pid_t pid;
	JobStatus status;
	int argc;
	char* cmdline;
} Job;

Job jobs[MAXJOBS];

int first_free_job();
int add_new_job (pid_t pid, char* cmdline);
int get_job_by_pid (pid_t pid);
void del_job (int i);
void print_jobs();
void handle_done();
void handler_sigchld(int sig);

#endif
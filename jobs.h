#ifndef __JOBS_H__
#define __JOBS_H__

#include "csapp.h"

#define MAXJOBS 12

typedef enum {
	FREE    = 1 << 0,
	STOPPED = 1 << 1,
	FG      = 1 << 2,
	BG      = 1 << 3,
	DONE    = 1 << 4,
} JobStatus;

typedef struct {
	pid_t pid;
	JobStatus status;
	int argc;
	char* cmdline;
} Job;

Job jobs[MAXJOBS];

void init_jobs();
int first_job_for_status(JobStatus status);
int first_free_job();
int get_fg();
int add_new_job (pid_t pid, char* cmdline);
int get_job_by_pid (pid_t pid);
void free_job (int jobid);
void print_job_status(int pid, char* status);
void print_jobs();
void handle_done();

#endif
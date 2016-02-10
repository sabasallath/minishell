#include "jobs.h"

Job jobs[MAXJOBS] = {{0}};

int first_free_job() {
	int i;
	for (i=0; i < MAXJOBS && jobs[i].status != FREE; i++);
	if (i == MAXJOBS) return -1;

	return i;
}

int add_new_job (pid_t pid, char* cmdline) {
	int i = first_free_job();
	if (i != -1) {
		int cmdline_size = strlen(cmdline) + 1;
		jobs[i].cmdline = malloc(cmdline_size * sizeof(char));
		if (jobs[i].cmdline == NULL) return -1;
		jobs[i].pid = pid;
		jobs[i].status = RUNNING;
		memcpy(jobs[i].cmdline, cmdline, cmdline_size);
	}

	return i;
}

int get_job_by_pid (pid_t pid) {
	int i;
	for (i=0; i < MAXJOBS && jobs[i].pid != pid; i++);
	if (i == MAXJOBS) return -1;

	return i;
}

void del_job (int jobid) {
	free(jobs[jobid].cmdline);
	jobs[jobid].status = FREE;
}

void print_jobs() {
	int i;
	for (i = 0; i < MAXJOBS; i++) {
		if (jobs[i].status == STOPPED || jobs[i].status == RUNNING) {
			printf("[%d] %s %s",
					i,
					jobs[i].status == STOPPED ? "STOPPED" : "RUNNING",
					jobs[i].cmdline);
		}
	}
}

void handle_done() {
	int i;
	for (i = 0; i < MAXJOBS; i++) {
		if (jobs[i].status == DONE) {
			printf("[%d] %s %s",
					i,
					"DONE",
					jobs[i].cmdline);
			del_job(i);
		}
	}
}

void handler_sigchld(int sig) {
    pid_t pid;

    while ((pid = waitpid(-1, NULL, WNOHANG)) > 0) {
    	int jobid = get_job_by_pid(pid);
    	jobs[jobid].status = DONE;
    }

    return;
}

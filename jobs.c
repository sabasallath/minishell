#include <stdio.h>
#include <string.h>
#include "jobs.h"

Job jobs[MAXJOBS] = {{0}};

int valid_jobid(jobid_t jobid) {
	if (jobid >= 0 && jobid < MAXJOBS) {
		return 1;
	}

	fprintf(stderr, "[Erreur] jobid %d is not valid\n", jobid);
	return 0;
}

void jobs_init () {
	jobid_t i;
	for (i=0; i < MAXJOBS; i++) {
		jobs[i].status = FREE;
	}
}

jobid_t jobs_find_first_by_status (JobStatus status) {
	jobid_t i = 0;
	while (i < MAXJOBS && !(jobs[i].status & status)) i++;
	return i == MAXJOBS ? INVALID_JOBID : i;
}

jobid_t jobs_find_by_pid (pid_t pid) {
	jobid_t i = 0;
	while (i < MAXJOBS && jobs[i].pid != pid) i++;
	return i == MAXJOBS ? INVALID_JOBID : i;
}

jobid_t jobs_add (pid_t pid, char* cmdline) {
	jobid_t jobid = jobs_find_first_by_status(FREE);
	Job* job = jobs + jobid;
	if (jobid != INVALID_JOBID) {
		int cmdline_size = strlen(cmdline) + 1;
		job->cmdline = malloc(cmdline_size * sizeof(char));
		if (job->cmdline == NULL) return INVALID_JOBID;
		memcpy(job->cmdline, cmdline, cmdline_size);

		job->pid = pid;
		job->status = BG;
	}

	return jobid;
}

void jobs_print (JobStatus status) {
	jobid_t i;
	for (i = 0; i < MAXJOBS; i++) {
		if (jobs[i].status & status) {
			job_print(i);
		}
	}
}

void jobs_handle_done () {
	jobid_t i;
	for (i = 0; i < MAXJOBS; i++) {
		if (jobs[i].status == DONE) {
			job_print(i);
			job_free(i);
		}
	}
}

void job_free (jobid_t jobid) {
	free(jobs[jobid].cmdline);
	jobs[jobid].status = FREE;
}

char* status_str(jobid_t jobid) {
	switch (jobs[jobid].status) {
		case FREE: return "Free";
		case STOPPED: return "Stopped";
		case FG: return "Running";
		case BG: return "Running";
		case DONE: return "Done";
	}

	return "Unkown";
}

void job_print (jobid_t jobid) {
	if (!valid_jobid(jobid)) return;
	job_print_with_status(jobid, status_str(jobid));
}

void job_print_with_status (jobid_t jobid, char* status) {
	if (!valid_jobid(jobid)) return;
	printf("[%d] %s %s", jobid, status, jobs[jobid].cmdline);
}

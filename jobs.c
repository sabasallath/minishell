#include <stdio.h>
#include <string.h>
#include "jobs.h"

Job jobs[MAXJOBS] = {{0}};

bool valid_jobid (jobid_t jobid) {
	if (jobid >= 0 && jobid < MAXJOBS) {
		return true;
	}

	fprintf(stderr, "[Erreur] jobid %d is not valid\n", jobid);
	return false;
}

void jobs_init () {
	jobid_t i;
	for (i=0; i < MAXJOBS; i++) {
		jobs[i].status = FREE;
	}
}

jobid_t jobs_find_first_by_status (JobStatus status) {
	jobid_t i = 0;
	while (i < MAXJOBS && !job_status_match(i, status)) i++;
	return i == MAXJOBS ? INVALID_JOBID : i;
}

jobid_t jobs_find_by_pid (pid_t pid) {
	jobid_t i = 0;
	while (i < MAXJOBS && jobs[i].pid != pid) i++;
	return i == MAXJOBS || job_status_match(i, FREE) ? INVALID_JOBID : i;
}

char* cmdline_copy(char* cmdline) {
	char* copy;
	// Ignore les espaces en début de chaîne
	while (*cmdline == ' ') cmdline++;

	int size = strlen(cmdline);

	int i = size - 1;
	// On ne veut pas conserver ces caractères en fin de chaîne
	while (cmdline[i] == '\n' || cmdline[i] == ' ' || cmdline[i] == '&') {
		i--;
	}
	size = i + 2;

	copy = malloc(size * sizeof(char));
	memcpy(copy, cmdline, size);
	copy[size - 1] = '\0';
	return copy;
}

jobid_t jobs_add (pid_t pid, char* cmdline) {
	jobid_t jobid = jobs_find_first_by_status(FREE);
	Job* job = jobs + jobid;
	if (jobid != INVALID_JOBID) {
		job->pid = pid;
		job->status = BG;
		job->cmdline = cmdline_copy(cmdline);
	}

	return jobid;
}

void jobs_print (JobStatus status) {
	jobid_t i;
	for (i = 0; i < MAXJOBS; i++) {
		if (job_status_match(i, status)) {
			job_print(i);
		}
	}
}

void jobs_free_done () {
	jobid_t i;
	for (i = 0; i < MAXJOBS; i++) {
		if (job_status_match(i, DONE)) {
			job_print(i);
			job_free(i);
		}
	}
}

void job_free (jobid_t jobid) {
	free(jobs[jobid].cmdline);
	jobs[jobid].status = FREE;
}

char* job_status_str (jobid_t jobid) {
	switch (jobs[jobid].status) {
		case FREE: return "Free";
		case STOPPED: return "Stopped";
		case FG: case BG: return "Running";
		case DONE: return "Done";
	}

	return "Unkown";
}

bool job_status_match (jobid_t jobid, JobStatus status) {
	return jobs[jobid].status & status;
}

void job_print (jobid_t jobid) {
	if (!valid_jobid(jobid)) return;
	job_print_with_status(jobid, job_status_str(jobid));
}

void job_print_with_pid (jobid_t jobid) {
	if (!valid_jobid(jobid)) return;
	printf("[%d] %d `%s`\n", jobid + 1, jobs[jobid].pid, jobs[jobid].cmdline);
}

void job_print_with_status (jobid_t jobid, char* status) {
	if (!valid_jobid(jobid)) return;
	printf("[%d] %s `%s`\n", jobid + 1, status, jobs[jobid].cmdline);
}

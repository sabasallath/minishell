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

void cmdline_copy(char* src, char dst[]) {
	// Ignore les espaces en début de chaîne
	while (*src == ' ') src++;

	int size = strlen(src);

	int i = size - 1;
	// On ne veut pas conserver ces caractères en fin de chaîne
	while (src[i] == '\n' || src[i] == ' ' || src[i] == '&') {
		i--;
	}
	size = i + 2;

	memcpy(dst, src, size);
	dst[size - 1] = '\0';
}

jobid_t jobs_add (pid_t pid, char* cmdline) {
	jobid_t jobid = jobs_find_first_by_status(FREE);
	Job* job = jobs + jobid;
	if (jobid != INVALID_JOBID) {
		job->pid = pid;
		job->status = BG;
		cmdline_copy(cmdline, job->cmdline);
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

void jobs_update () {
	jobid_t i;
	for (i = 0; i < MAXJOBS; i++) {
		if (job_status_match(i, UPDATED)) {
			job_update(i, true);
		}
	}
}

void job_update(jobid_t jobid, bool print_ifexited) {
	int status = jobs[jobid].updated_status;
	if (WIFEXITED(status)) {
		jobs[jobid].status = FREE;
		if (print_ifexited) job_print_with_status(jobid, "Done");
	}
	else if (WIFSIGNALED(status)) {
		jobs[jobid].status = FREE;
		job_print_with_status(jobid, strsignal(WTERMSIG(status)));
	}
	else if (WIFSTOPPED(status)) {
		jobs[jobid].status = STOPPED;
		job_print_with_status(jobid, "Stopped");
	}
	else if (WIFCONTINUED(status)) {
		jobs[jobid].status = BG;
		job_print_with_status(jobid, "Continued");
	}
}

char* job_status_str (jobid_t jobid) {
	switch (jobs[jobid].status) {
		case FREE: return "Free";
		case STOPPED: return "Stopped";
		case FG: case BG: return "Running";
		case UPDATED: return "Updated";
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

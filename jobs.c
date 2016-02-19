#include "jobs.h"
#include "signals.h"

/////////////////////////////////////////////////////
// Type et constantes
/////////////////////////////////////////////////////

typedef enum {
	NO            = 0,
	YES           = 1,
	YES_WHILE_FG  = 2,
} Updated;

typedef struct {
    pid_t pid;
    JobStatus status;
    char cmdline[MAXLINE];

    Updated updated;
    int wait_status;
} Job;

Job jobs[MAXJOBS] = {{0}};

/////////////////////////////////////////////////////
// Fonctions utilitaires privées
/////////////////////////////////////////////////////

// Verifie que le jobid est dans l'interval attendu
bool valid_jobid (jobid_t jobid) {
	if (jobid >= 0 && jobid < MAXJOBS) {
		return true;
	}

	fprintf(stderr, "[Erreur] jobid %d is not valid\n", jobid);
	return false;
}

/////////////////////////////////////////////////////
// Fonctions de gestion des jobs
/////////////////////////////////////////////////////

void jobs_init () {
	jobid_t i;
	for (i=0; i < MAXJOBS; i++) {
		jobs[i].status = FREE;
	}
}

void cmdline_copy (char* src, char dst[]) {
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
		job->status = RUNNING;
		cmdline_copy(cmdline, job->cmdline);
	}

	return jobid;
}

int job_kill (jobid_t jobid, int sig) {
	return kill(jobs[jobid].pid, sig);
}

void job_change_status (jobid_t jobid, int sig) {
	bool continue_before = sig != SIGCONT && job_status_match(jobid, STOPPED);

	signals_unlock();
	if (continue_before)
		job_kill(jobid, SIGCONT);
    job_kill(jobid, sig);

    // On attends un peu pour laisser le temps au signal d'avoir un effet
    // Certain signaux peuvent avoir été redefini par le processus enfant
    // et ne pas avoir l'effet par défaut.
    // Il n'est donc pas envisageable d'attendre indéfiniment. Tant pis.
    int time = 1;
    while ((time = sleep(time)) > 0) {
    	if (jobs[jobid].updated && (!continue_before || !job_status_match(jobid, RUNNING))) {
    		break;
    	}
    }
	signals_lock();

    job_print_update(jobid);
}

void job_fg_wait (jobid_t jobid) {
	if (!valid_jobid(jobid)) return;
	if (!job_status_match(jobid, RUNNING)) return;
	jobs[jobid].status |= FG;

	signals_unlock();
    while (job_status_match(jobid, RUNNING))
        sleep(0);
    signals_lock();

    job_print_update(jobid);
}

void job_update (jobid_t jobid, int wait_status) {
	if (valid_jobid(jobid)) {
		jobs[jobid].updated = jobs[jobid].status & FG ? YES_WHILE_FG : YES;
		jobs[jobid].wait_status = wait_status;

		if (WIFEXITED(wait_status)) {
			jobs[jobid].status = DONE;
		}
		else if (WIFSIGNALED(wait_status)) {
			jobs[jobid].status = DONE;
		}
		else if (WIFSTOPPED(wait_status)) {
			jobs[jobid].status = STOPPED;
		}
		else if (WIFCONTINUED(wait_status)) {
			jobs[jobid].status = RUNNING;
		}
	}
}

void job_do_print_update (jobid_t jobid) {
	bool while_fg = jobs[jobid].updated == YES_WHILE_FG;
	int wait_status = jobs[jobid].wait_status;
	jobs[jobid].updated = NO;
	jobs[jobid].wait_status = 0;

	if (WIFEXITED(wait_status)) {
		jobs[jobid].status = FREE;
		if (!while_fg) job_print_with_status(jobid, "Done");
	}
	else if (WIFSIGNALED(wait_status)) {
		jobs[jobid].status = FREE;
		if (!while_fg || WTERMSIG(wait_status) != SIGINT)
			job_print_with_status(jobid, strsignal(WTERMSIG(wait_status)));
	}
	else if (WIFSTOPPED(wait_status)) {
		job_print_with_status(jobid, "Stopped");
	}
	else if (WIFCONTINUED(wait_status)) {
		job_print_with_status(jobid, "Continued");
	}
}

void jobs_print_update () {
	jobid_t i;
	for (i = 0; i < MAXJOBS; i++) {
		if (jobs[i].updated) {
			job_do_print_update(i);
		}
	}
}

void job_print_update (jobid_t jobid) {
	if (valid_jobid(jobid) && jobs[jobid].updated)
		job_do_print_update(jobid);
}

/////////////////////////////////////////////////////
// Fonctions utilitaires pour manipuler les jobs
/////////////////////////////////////////////////////

pid_t job_pid (jobid_t jobid) {
	return !valid_jobid(jobid) || job_status_match(jobid, FREE)
			? 0
			: jobs[jobid].pid;
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

bool job_status_match (jobid_t jobid, JobStatus status) {
	if (!valid_jobid(jobid)) return false;
	return jobs[jobid].status & status;
}

void jobs_print (JobStatus status, bool updated) {
	jobid_t i;
	for (i = 0; i < MAXJOBS; i++) {
		if (job_status_match(i, status) && (!jobs[i].updated || updated)) {
			job_print(i);
		}
	}
}

char* job_status_str (jobid_t jobid) {
	if (!valid_jobid(jobid)) return "Invalid";

	if (job_status_match(jobid, FREE))    return "Free";
	if (job_status_match(jobid, STOPPED)) return "Stopped";
	if (job_status_match(jobid, RUNNING)) return "Running";
	if (job_status_match(jobid, DONE))    return "Done";

	return "Unkown";
}

void job_print (jobid_t jobid) {
	if (valid_jobid(jobid))
		job_print_with_status(jobid, job_status_str(jobid));
}

void job_print_with_pid (jobid_t jobid) {
	if (valid_jobid(jobid))
		printf("[%d]  %d\t%s\n", jobid + 1, jobs[jobid].pid, jobs[jobid].cmdline);
}

void job_print_with_status (jobid_t jobid, char* status) {
	if (valid_jobid(jobid))
		printf("[%d]  %s\t%s\n", jobid + 1, status, jobs[jobid].cmdline);
}

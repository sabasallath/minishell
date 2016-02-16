#include "jobs.h"

/////////////////////////////////////////////////////
// Type et constantes
/////////////////////////////////////////////////////

typedef struct {
    pid_t pid;
    JobStatus status;
    int argc;
    char cmdline[MAXLINE];
    int updated_status;
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
		job->status = RUNNING;
		cmdline_copy(cmdline, job->cmdline);
	}

	return jobid;
}

void job_kill(jobid_t jobid, int sig) {
	Kill(jobs[jobid].pid, sig);
}

void job_change_status(jobid_t jobid, int sig) {
	if (job_status_match(jobid, STOPPED) && sig != SIGCONT)
		job_kill(jobid, SIGCONT);
    job_kill(jobid, sig);

    // On attends un peu pour laisser le temps au signal d'avoir un effet
    sleep(1);

    // Certain signaux peuvent avoir été redefini par le processus enfant
    // et ne pas avoir l'effet par défaut.
    // Il n'est donc pas envisageable d'attendre indéfiniment. Tant pis.
    if (job_status_match(jobid, UPDATED))
        job_update(jobid);
}

void job_fg_wait(jobid_t jobid) {
	if (!valid_jobid(jobid)) return;
	if (!job_status_match(jobid, RUNNING)) return;

	jobs[jobid].status |= FG;
    while (job_status_match(jobid, RUNNING))
        sleep(0);

    if (job_status_match(jobid, UPDATED))
        job_update(jobid);
}

void job_updated(jobid_t jobid, int updated_status) {
	if (!valid_jobid(jobid)) return;

	jobs[jobid].status &= FG;
	jobs[jobid].status |= UPDATED;
	jobs[jobid].updated_status = updated_status;
}

void jobs_update () {
	jobid_t i;
	for (i = 0; i < MAXJOBS; i++) {
		if (job_status_match(i, UPDATED)) {
			job_update(i);
		}
	}
}

void job_update(jobid_t jobid) {
	if (!valid_jobid(jobid)) return;

	bool was_fg = jobs[jobid].status & FG;
	int status = jobs[jobid].updated_status;
	if (WIFEXITED(status)) {
		jobs[jobid].status = FREE;
		if (!was_fg) job_print_with_status(jobid, "Done");
	}
	else if (WIFSIGNALED(status)) {
		jobs[jobid].status = FREE;
		if (!was_fg || WTERMSIG(status) != SIGINT)
			job_print_with_status(jobid, strsignal(WTERMSIG(status)));
	}
	else if (WIFSTOPPED(status)) {
		jobs[jobid].status = STOPPED;
		job_print_with_status(jobid, "Stopped");
	}
	else if (WIFCONTINUED(status)) {
		jobs[jobid].status = RUNNING;
		job_print_with_status(jobid, "Continued");
	}
}

/////////////////////////////////////////////////////
// Fonctions utilitaires pour manipuler les jobs
/////////////////////////////////////////////////////

pid_t job_pid(jobid_t jobid) {
	if (!valid_jobid(jobid)) return 0;
	return job_status_match(jobid, FREE)
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

void jobs_print (JobStatus status) {
	jobid_t i;
	for (i = 0; i < MAXJOBS; i++) {
		if (job_status_match(i, status)) {
			job_print(i);
		}
	}
}

char* job_status_str (jobid_t jobid) {
	if (!valid_jobid(jobid)) return "Invalid";

	if (job_status_match(jobid, FREE)) return "Free";
	if (job_status_match(jobid, STOPPED)) return "Stopped";
	if (job_status_match(jobid, RUNNING)) return "Running";
	if (job_status_match(jobid, UPDATED)) return "Updated";

	return "Unkown";
}

void job_print (jobid_t jobid) {
	if (valid_jobid(jobid))
		job_print_with_status(jobid, job_status_str(jobid));
}

void job_print_with_pid (jobid_t jobid) {
	if (valid_jobid(jobid))
		printf("[%d] %d `%s`\n", jobid + 1, jobs[jobid].pid, jobs[jobid].cmdline);
}

void job_print_with_status (jobid_t jobid, char* status) {
	if (valid_jobid(jobid))
		printf("[%d] %s `%s`\n", jobid + 1, status, jobs[jobid].cmdline);
}

/////////////////////////////////////////////////////
// Securisation des operations sur les jobs
/////////////////////////////////////////////////////

void interrupt_lock(Sigmask sigmask) {
    Sigemptyset(&sigmask.current);
    Sigemptyset(&sigmask.saved);
    Sigaddset(&sigmask.current, SIGCHLD);
    Sigaddset(&sigmask.current, SIGINT);
    Sigaddset(&sigmask.current, SIGTSTP);
    Sigprocmask(SIG_BLOCK, &sigmask.current, &sigmask.saved);
}

void interrupt_unlock(Sigmask sigmask) { 
	Sigemptyset(&sigmask.pending);
	sigpending(&sigmask.pending);
    Sigprocmask(SIG_SETMASK, &sigmask.saved, &sigmask.pending);
}
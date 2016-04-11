#include "jobs.h"
#include "terminal.h"

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

	TerminalControl terminal_ctl;

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

#define status_match(jobid, status_p) (jobs[jobid].status & (status_p))

/////////////////////////////////////////////////////
// Fonctions pour les signaux (voir signals.c)
/////////////////////////////////////////////////////

void jobs_signals_init ();
void jobs_signals_lock ();
void jobs_signals_unlock ();

/////////////////////////////////////////////////////
// Fonctions de gestion des jobs
/////////////////////////////////////////////////////

void jobs_init () {
	jobid_t i;
	for (i=0; i < MAXJOBS; i++) {
		jobs[i].status = FREE;
	}

	jobs_signals_init();
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
	jobs_signals_lock();
	jobid_t jobid = jobs_find_first_by_status(FREE);
	Job* job = jobs + jobid;
	if (jobid != INVALID_JOBID) {
		job->pid = pid;
		job->status = RUNNING;
		terminal_init_control(&job->terminal_ctl);
		cmdline_copy(cmdline, job->cmdline);
	}

	jobs_signals_unlock();
	return jobid;
}

int job_kill (jobid_t jobid, int sig) {
	return kill(jobs[jobid].pid, sig);
}

void job_change_status (jobid_t jobid, int sig) {
	jobs_signals_lock();

	bool continue_before = status_match(jobid, STOPPED)
			&& sig != SIGCONT
			&& sig != SIGSTOP
			&& sig != SIGTSTP;
	if (continue_before)
		job_kill(jobid, SIGCONT);

    job_kill(jobid, sig);

    jobs_signals_unlock();

    // On attends un peu pour laisser le temps au signal d'avoir un effet
    // Certain signaux peuvent avoir été redéfini par le processus enfant
    // et ne pas avoir l'effet par défaut.
    // Il n'est donc pas envisageable d'attendre indéfiniment. Tant pis.
    int time = 1;
    while ((time = sleep(time)) > 0) {
    	// Si sleep s'arrete avant le temps imparti, c'est potentiellement
    	// parce que le changement de statut attendu est effectué.
    	// On vérifie donc si c'est le cas pour pouvoir rendre la main au
    	// plus tôt.
    	if (jobs[jobid].updated && (!continue_before || !status_match(jobid, RUNNING))) {
    		break;
    	}
    }

    job_print_update(jobid);
}

void job_fg_wait (jobid_t jobid, bool print) {
	if (!valid_jobid(jobid)) return;
	if (!status_match(jobid, RUNNING | STOPPED)) return;

	jobs_signals_lock();

	terminal_give_control(jobs[jobid].pid, &jobs[jobid].terminal_ctl);

	if (status_match(jobid, STOPPED))
		// Si le job est stoppé, on doit commencer par essayer de le
		// relancer
		job_change_status(jobid, SIGCONT);
	else if (print)
		job_print(jobid);

	jobs[jobid].status |= FG;
	jobs_signals_unlock();

	while (status_match(jobid, RUNNING))
        sleep(1);

    terminal_take_back_control(&jobs[jobid].terminal_ctl);
    job_print_update(jobid);
}

void job_update (jobid_t jobid, int wait_status) {
	if (valid_jobid(jobid)) {
		jobs[jobid].updated = jobs[jobid].status & FG
				? YES_WHILE_FG
				: YES;
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
		// On ne signale pas un arrêt par un SIGINT par un message
		// si le job était au premier plan
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
	jobs_signals_lock();
	for (i = 0; i < MAXJOBS; i++) {
		if (jobs[i].updated) {
			job_do_print_update(i);
		}
	}
	jobs_signals_unlock();
}

void job_print_update (jobid_t jobid) {
	jobs_signals_lock();
	if (valid_jobid(jobid) && jobs[jobid].updated) {
		job_do_print_update(jobid);
	}
	jobs_signals_unlock();
}

/////////////////////////////////////////////////////
// Fonctions utilitaires pour manipuler les jobs
/////////////////////////////////////////////////////

pid_t job_pid (jobid_t jobid) {
	return !valid_jobid(jobid) || status_match(jobid, FREE)
			? 0
			: jobs[jobid].pid;
}

jobid_t jobs_find_first_by_status (JobStatus status) {
	jobid_t i = 0;
	while (i < MAXJOBS && !status_match(i, status)) i++;
	return i == MAXJOBS ? INVALID_JOBID : i;
}

jobid_t jobs_find_by_pid (pid_t pid) {
	jobid_t i = 0;
	while (i < MAXJOBS && jobs[i].pid != pid) i++;
	return i == MAXJOBS || status_match(i, FREE) ? INVALID_JOBID : i;
}

bool job_status_match (jobid_t jobid, JobStatus status) {
	return valid_jobid(jobid) && status_match(jobid, status);
}

void jobs_print (JobStatus status, bool updated) {
	jobid_t i;
	for (i = 0; i < MAXJOBS; i++) {
		if (status_match(i, status) && (!jobs[i].updated || updated)) {
			job_print(i);
		}
	}
}

char* job_status_str (jobid_t jobid) {
	if (!valid_jobid(jobid)) return "Invalid";

	if (status_match(jobid, FREE))    return "Free";
	if (status_match(jobid, STOPPED)) return "Stopped";
	if (status_match(jobid, RUNNING)) return "Running";
	if (status_match(jobid, DONE))    return "Done";

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

#include "minishell.h"
#include "jobs.h"

void handler_sigint (int sig) {
    jobid_t jobid = jobs_find_first_by_status(FG);
    if (jobid != INVALID_JOBID) {
        Kill(jobs[jobid].pid, SIGINT);
        printf("\n");
    }
}

void handler_sigtstp (int sig) {
    jobid_t jobid = jobs_find_first_by_status(FG);
    if (jobid != INVALID_JOBID) {
        Kill(jobs[jobid].pid, SIGSTOP);
        jobs[jobid].status = STOPPED;
        printf("\n");
    }
}

void handler_sigchld (int sig) {
    pid_t pid;
    int status;

    while ((pid = waitpid(-1, &status, WNOHANG|WCONTINUED|WUNTRACED)) > 0) {
        jobid_t jobid = jobs_find_by_pid(pid);
        if (jobid == INVALID_JOBID) {
            printf("Got a SIGCHLD for child with pid %d, but no corresponding job found\n", pid);
            continue;
        }

        // Le status UPDATED est un statut intermédiaire indiquant qu'il faut
        // mettre à jour le status du job et indiquer le changement à l'utilisateur.
        jobs[jobid].status = UPDATED;
        jobs[jobid].updated_status = status;
    }
}

void shell_signals () {
    Signal(SIGCHLD, handler_sigchld);
    Signal(SIGINT, handler_sigint);
    Signal(SIGTSTP, handler_sigtstp);
}

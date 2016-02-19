#include "minishell.h"
#include "jobs.h"
#include "signals.h"

bool waiting;

void handler_sigint (int sig) {
    jobid_t jobid = jobs_find_first_by_status(FG);
    if (waiting) {
        tty_printf("\n"); // Retour a la ligne après le symbole de controle
        waiting = false;
    }
    else if (jobid != INVALID_JOBID) {
        tty_printf("\n"); // Retour a la ligne après le symbole de controle
        if (job_status_match(jobid, STOPPED))
            job_kill(jobid, SIGCONT);
        job_kill(jobid, SIGINT);
    }
}

void handler_sigtstp (int sig) {
    jobid_t jobid = jobs_find_first_by_status(FG);
    if (jobid != INVALID_JOBID) {
        tty_printf("\n"); // Retour a la ligne après le symbole de controle
        job_kill(jobid, SIGSTOP);
    }
}

void handler_sigchld (int sig) {
    pid_t pid;
    int status;

    while ((pid = waitpid(-1, &status, WNOHANG | WCONTINUED | WUNTRACED)) > 0) {
        jobid_t jobid = jobs_find_by_pid(pid);
        if (jobid == INVALID_JOBID) {
            printf("Got a SIGCHLD for child with pid %d, but no corresponding job found\n", pid);
            continue;
        }

        // On indique que le status du job doit être mise à jour
        job_update(jobid, status);
    }
}

typedef struct {
    bool locked;
    sigset_t current;
} Lock;

Lock lock;

void signals_init () {
    Signal(SIGCHLD, handler_sigchld);
    Signal(SIGINT, handler_sigint);
    Signal(SIGTSTP, handler_sigtstp);

    Sigemptyset(&lock.current);
    Sigaddset(&lock.current, SIGCHLD);
    Sigaddset(&lock.current, SIGINT);
    Sigaddset(&lock.current, SIGTSTP);
}

void signals_lock (char* desc) {
    if (!lock.locked) {
        lock.locked = true;
        Sigprocmask(SIG_BLOCK, &lock.current, NULL);
    }
}

void signals_unlock (char* desc) {
    if (lock.locked) {
        lock.locked = false;
        Sigprocmask(SIG_UNBLOCK, &lock.current, NULL);
    }
}

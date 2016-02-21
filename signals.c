#include "minishell.h"
#include "jobs.h"
#include "terminal.h"
#include "signals.h"

bool waiting;

void handler_sigint (int sig) {
    if (waiting) {
        terminal_printf("\n"); // Retour a la ligne après le symbole de controle
        waiting = false;
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
    Signal(SIGTSTP, SIG_IGN);

    Sigemptyset(&lock.current);
    Sigaddset(&lock.current, SIGCHLD);
    Sigaddset(&lock.current, SIGINT);
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

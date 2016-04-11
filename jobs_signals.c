#include "minishell.h"
#include "jobs.h"
#include "terminal.h"

bool builtin_waiting;

void handler_sigint (int sig) {
    if (builtin_waiting) {
        terminal_printf("\n"); // Retour a la ligne après le symbole de controle
        builtin_waiting = false;
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

sigset_t signals_to_block;

// Initialise la gestion des signaux utilisées par le shell
// (assigne les handlers et prépare les bloquages)
void jobs_signals_init () {
    Signal(SIGCHLD, handler_sigchld);
    Signal(SIGINT, handler_sigint);
    Signal(SIGTSTP, SIG_IGN);

    Sigemptyset(&signals_to_block);
    Sigaddset(&signals_to_block, SIGCHLD);
}

// Bloque les signaux importants pour le shell s'ils ne sont
// pas déjà bloqués
void jobs_signals_lock (char* desc) {
    Sigprocmask(SIG_BLOCK, &signals_to_block, NULL);
}

// Débloque les signaux importants pour le shell s'ils ont
// été bloqués au préalable par un appel à `signals_lock`
void jobs_signals_unlock (char* desc) {
    Sigprocmask(SIG_UNBLOCK, &signals_to_block, NULL);
}

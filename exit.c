#include "minishell.h"
#include "jobs.h"
#include "exit.h"

static int exit_next_forced = 0;

void try_terminate_jobs () {
    // On essaie de signaler aux jobs en cours qu'il devrait essayer de se terminer gentiment.
    jobid_t i;
    for (i = 0; i < MAXJOBS; i++) {
        if (!job_status_match(i, FREE)) {
            if (job_status_match(i, STOPPED))
                Kill(jobs[i].pid, SIGCONT);
            Kill(jobs[i].pid, SIGTERM);
        }
    }

    sleep(2);          // On attends un peu que les jobs aient le temps de se terminer
    jobs_update();     // Traite et affiche les jobs terminés
    jobs_print(~FREE); // Signal les jobs non terminés a l'utilisateur
}

void exit_try () {
    jobs_update();

    jobid_t jobid = jobs_find_first_by_status(~FREE);
    if (jobid == INVALID_JOBID) {
        exit(0);
    }

    if (exit_next_forced > 0) {
        try_terminate_jobs();
        exit(0);
    }
    else {
        printf("You have some jobs left (run again to force exit)\n");
        // Valeur "2" parce qu'on va l'"oublié" une première fois avant la prochaine execution
        exit_next_forced = 2;
    }
}

void exit_force () {
    exit_next_forced = 1;
    exit_try();
}

void exit_forget_next_forced () {
    exit_next_forced--;
}
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
                job_kill(i, SIGCONT);
            job_kill(i, SIGTERM);
        }
    }

    // On attends un peu que les jobs aient le temps de se terminer
    int time = 3;
    while ((time = sleep(time)) > 0) {
        jobid_t jobid = jobs_find_first_by_status(~(FREE | DONE));
        if (jobid == INVALID_JOBID) // Tous les jobs terminés
            break;
    }

    jobs_print(~FREE, false);  // Signal les jobs non terminés à l'utilisateur
    jobs_print_update();       // Traite et affiche les jobs terminés
}

void exit_try () {
    jobid_t jobid = jobs_find_first_by_status(~(FREE | DONE));
    if (jobid == INVALID_JOBID) {
        jobs_print_update();
        exit(0);
    }

    if (exit_next_forced > 0) {
        try_terminate_jobs();
        exit(0);
    }
    else {
        printf("You have some jobs left (run again to force exit)\n");
        // Valeur "2" parce qu'on va l'"oublié" une première fois
        // avant la prochaine execution
        exit_next_forced = 2;
    }
}

void exit_force () {
    exit_next_forced = 1;
    exit_try();
}

void exit_forget_next_forced () {
    if (exit_next_forced > 0)
        exit_next_forced--;
}
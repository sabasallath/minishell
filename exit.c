#include "minishell.h"
#include "jobs.h"
#include "exit.h"

static int exit_next_forced = 0;

void exit_try () {
    jobid_t jobid = jobs_find_first_by_status(~(FREE | UPDATED));
    if (jobid == INVALID_JOBID) {
        exit(0);
    }

    if (exit_next_forced > 0) {
        exit_force();
    }
    else {
        printf("You have some jobs left (run again to force exit)\n");
        // Valeur "2" parce qu'on va l'"oublié" une première fois avant la prochaine execution
        exit_next_forced = 2;
    }
}

void exit_force () {
    // On essaie de signaler au job en cours qu'il devrait essayer de se terminer gentiment.
    jobid_t i;
    for (i = 0; i < MAXJOBS; i++) {
        if (!(jobs[i].status & (FREE | UPDATED))) {
            if (jobs[i].status & STOPPED)
                Kill(jobs[i].pid, SIGCONT);
            Kill(jobs[i].pid, SIGTERM);
        }
    }


    sleep(2); // On attends un peu que les jobs aient le temps de se terminer
    jobs_update();
    jobs_print(~(FREE | UPDATED)); // Signal les jobs non terminées a l'utilisateur

    exit(0);
}

void exit_forget_next_forced () {
    exit_next_forced--;
}
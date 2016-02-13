#include "csapp.h"
#include "builtin.h"

jobid_t parse_jobid (char* arg) {
    if (*arg == '\0') {
        return INVALID_JOBID;
    }

	char* end = arg;
    jobid_t jobid = strtol(arg, &end, 10);
    if (*end != '\0') {
        // L'argument ne contient pas que des digits
        return INVALID_JOBID;
    }

    if (jobid <= 0 || jobid > MAXJOBS) {
    	return INVALID_JOBID;
    }

    return jobid - 1;
}

jobid_t read_jobid (char** argv, JobStatus status) {
    if (argv[1] == NULL) { // Pas d'argument spécifié pour la commande
        // Cherche un jobid par défaut.
        jobid_t jobid = jobs_find_first_by_status(status);
        if (jobid == INVALID_JOBID) {
            fprintf(stderr, "No default job found for command `%s`\n", argv[0]);
            return INVALID_JOBID;
        }

        return jobid;
    }

    jobid_t jobid = parse_jobid(argv[1]);
    if (jobid == INVALID_JOBID) {
        fprintf(stderr, "Wrong jobid `%s`, expected number between 1 and %d\n",
                argv[1], MAXJOBS);
        return INVALID_JOBID;
    }
    if (job_status_match(jobid, FREE)) {
        fprintf(stderr, "No job with id `%d`\n", jobid);
        return INVALID_JOBID;
    }
    if (!job_status_match(jobid, status)) {
        fprintf(stderr, "Can't use command `%s` for job with status `%s`\n",
                argv[0], job_status_str(jobid));
        return INVALID_JOBID;
    }

    return jobid;
}

void fg (jobid_t jobid) {
    if (job_status_match(jobid, STOPPED))
        bg(jobid);
    else
        job_print(jobid);
    fg_wait(jobid);
}

void fg_wait (jobid_t jobid) {
    jobs[jobid].status = FG;
    while (job_status_match(jobid, FG)) {
        sleep(0);
    }

    if (job_status_match(jobid, DONE)) {
        // On libère directement le job pour eviter de
        // print le job en tant que Done.
        // Rendre la main à l'utilisateur est suffisant
        // pour le lui signaler.
        job_free(jobid);
    }
}

void bg (jobid_t jobid) {
    Kill(jobs[jobid].pid, SIGCONT);
    jobs[jobid].status = BG;
    job_print_with_status(jobid, "Continued");
}

void interrupt (jobid_t jobid) {
    Kill(jobs[jobid].pid, SIGINT);
    // On attends un peu pour laisser le temps au job de se terminer
    sleep(1);
    // Si le signal cause la terminaison, un message sera affiché
    // automatiquement grâce au handler de SIGCHLD
}

void stop (jobid_t jobid) {
    Kill(jobs[jobid].pid, SIGSTOP);
    jobs[jobid].status = STOPPED;
    job_print_with_status(jobid, "Stopped");
}

void term (jobid_t jobid) {
    Kill(jobs[jobid].pid, SIGTERM);
    // On attends un peu pour laisser le temps au job de se terminer
    sleep(1);
    // Si le signal cause la terminaison, un message sera affiché
    // automatiquement grâce au handler de SIGCHLD
}

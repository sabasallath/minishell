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

pid_t parse_pid(char* arg) {
    if (*arg == '\0') {
        return 0;
    }

    char* end = arg;
    pid_t pid = strtol(arg, &end, 10);
    return *end != '\0'
            ? 0 // L'argument ne contient pas que des digits
            : pid;
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

    jobid_t jobid;
    if (argv[1][0] == '%') {
        jobid = parse_jobid(argv[1] + 1);
        if (jobid == INVALID_JOBID) {
            fprintf(stderr, "Wrong jobid `%s`, expected number between 1 and %d\n",
                    argv[1] + 1, MAXJOBS);
            return INVALID_JOBID;
        }

        if (job_status_match(jobid, FREE)) {
            fprintf(stderr, "No job with id `%d`\n", jobid + 1);
            return INVALID_JOBID;
        }
    }
    else {
        pid_t pid = parse_pid(argv[1]);
        if (pid == 0) {
            fprintf(stderr, "Invalid pid `%s`\n", argv[1]);
            return INVALID_JOBID;
        }

        jobid = jobs_find_by_pid(pid);

        if (jobid == INVALID_JOBID) {
            fprintf(stderr, "No job found for pid `%d`\n", pid);
            return INVALID_JOBID;
        }
    }

    if (!job_status_match(jobid, status)) {
        fprintf(stderr, "Can't use command `%s` for job with status `%s`\n",
                argv[0], job_status_str(jobid));
        return INVALID_JOBID;
    }

    return jobid;
}

void job_change_status(jobid_t jobid, int sig) {
    Kill(jobs[jobid].pid, sig);

    // On attends un peu pour laisser le temps au signal d'avoir un effet
    sleep(1);

    // Certain signaux peuvent avoir été redefini par le processus enfant
    // et ne pas avoir l'effet par défaut.
    // Il n'est donc pas envisageable d'attendre indéfiniment. Tant pis.
    if (job_status_match(jobid, UPDATED))
        job_update(jobid, true);
}

void fg (jobid_t jobid) {
    if (job_status_match(jobid, STOPPED))
        job_change_status(jobid, SIGCONT);
    else
        job_print(jobid);

    fg_wait(jobid);
}

void fg_wait (jobid_t jobid) {
    jobs[jobid].status = FG;
    while (job_status_match(jobid, FG)) {
        sleep(0);
    }

    if (job_status_match(jobid, UPDATED)) {
        job_update(jobid, false);
    }
}

void bg (jobid_t jobid) {
    job_change_status(jobid, SIGCONT);
}

void interrupt (jobid_t jobid) {
    job_change_status(jobid, SIGINT);
}

void stop (jobid_t jobid) {
    job_change_status(jobid, SIGSTOP);
}

void term (jobid_t jobid) {
    job_change_status(jobid, SIGTERM);
}

void builtin_wait () {
    jobid_t jobid;
    while ((jobid = jobs_find_first_by_status(~(FREE | STOPPED))) != INVALID_JOBID) {
        sleep(0);
        jobs_update();
    }
}
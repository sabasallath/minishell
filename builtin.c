#include "csapp.h"
#include "builtin.h"

jobid_t parse_jobid (char* arg) {
    if (*arg == '\0') {
        return INVALID_JOBID;
    }

	char* end = arg;
    jobid_t jobid = strtol(arg, &end, 10);
    if (*end != '\0') {
        return INVALID_JOBID;
    }

    if (jobid >= MAXJOBS) {
    	return INVALID_JOBID;
    }

    return jobid;
}

jobid_t read_jobid (char** argv, JobStatus status) {
    if (argv[1] == NULL) {
        jobid_t jobid = jobs_find_first_by_status(status);
        if (jobid == INVALID_JOBID) {
            fprintf(stderr, "No default job\n");
            return INVALID_JOBID;
        }

        return jobid;
    }

    jobid_t jobid = parse_jobid(argv[1]);
    if (jobid == INVALID_JOBID) {
        fprintf(stderr, "Wrong jobid: %s, expected number between 0 and %d\n", argv[1], MAXJOBS - 1);
        return INVALID_JOBID;
    }
    if (jobs[jobid].status == FREE) {
        fprintf(stderr, "No such job: %d\n", jobid);
        return INVALID_JOBID;
    }
    if (!(jobs[jobid].status & status)) {
        fprintf(stderr, "You can't do that with this job\n");
        return INVALID_JOBID;
    }

    return jobid;
}

void fg (jobid_t jobid) {
    if (jobs[jobid].status == STOPPED) {
        bg(jobid);
    }

    jobs[jobid].status = FG;
    while (jobs[jobid].status == FG) {
        sleep(0);
    }

    if (jobs[jobid].status == DONE) {
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
    sleep(1);
}

void stop (jobid_t jobid) {
    Kill(jobs[jobid].pid, SIGSTOP);
    jobs[jobid].status = STOPPED;
    job_print_with_status(jobid, "Stopped");
}

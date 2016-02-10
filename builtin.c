#include "csapp.h"
#include "builtin.h"

int parse_jobid (char* arg) {
    if (*arg == '\0') {
        return -1;
    }

	char* end = arg;
    int jobid = strtol(arg, &end, 10);
    if (*end != '\0') {
        return -1;
    }

    if (jobid >= MAXJOBS) {
    	return -1;
    }

    return jobid;
}

int read_jobid (char** argv, JobStatus status) {
    if (argv[1] == NULL) {
        int jobid = first_job_for_status(status);
        if (jobid == -1) {
            fprintf(stderr, "No default job\n");
            return -1;
        }

        return jobid;
    }

    int jobid = parse_jobid(argv[1]);
    if (jobid == -1) {
        fprintf(stderr, "Wrong jobid: %s, expected number between 0 and %d\n", argv[1], MAXJOBS - 1);
        return -1;
    }
    if (jobs[jobid].status == FREE) {
        fprintf(stderr, "No such job: %d\n", jobid);
        return -1;
    }
    if (!(jobs[jobid].status & status)) {
        fprintf(stderr, "You can't do that with this job\n");
        return -1;
    }

    return jobid;
}

void fg (int jobid) {
    if (jobs[jobid].status == STOPPED) {
        bg(jobid);
    }

    jobs[jobid].status = FG;
    while (jobs[jobid].status == FG) {
        sleep(0);
    }

    if (jobs[jobid].status == DONE) {
        free_job(jobid);
    }
}

void bg (int jobid) {
    Kill(jobs[jobid].pid, SIGCONT);
    jobs[jobid].status = BG;
    print_job_status(jobid, "Continued");
}

void stop (int jobid) {
    Kill(jobs[jobid].pid, SIGSTOP);
    jobs[jobid].status = STOPPED;
    print_job_status(jobid, "Stopped");
}

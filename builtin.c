#include "csapp.h"
#include "builtin.h"
#include "jobs.h"


void fg (int jobid) {
    if (jobs[jobid].status == STOPPED) {
        Kill(jobs[jobid].pid, SIGCONT);
    }

    jobs[jobid].status = FG;
	while (jobs[jobid].status == FG) {
	    sleep(0);
	}

	if (jobs[jobid].status == DONE) {
		free_job(jobid);
	}
}

int read_jobid (char* arg) {
    if (*arg == '\0') {
        return -1;
    }

	char* end = arg;
    int jobid = strtol(arg, &end, 16);
    if (*end != '\0') {
        return -1;
    }

    if (jobid > MAXJOBS) {
    	return -1;
    }

    return jobid;
}

int default_fg_jobid () {
	return first_job_for_status(BG | STOPPED);
}
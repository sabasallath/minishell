#include "myshell.h"
#include "jobs.h"
#include "handlers.h"

void handler_sigint (int sig) {
    jobid_t jobid = jobs_find_first_by_status(FG);
    if (jobid != INVALID_JOBID) {
        Kill(jobs[jobid].pid, SIGINT);
        printf("\n");
    }
}

void handler_sigtstp (int sig) {
    jobid_t jobid = jobs_find_first_by_status(FG);
    if (jobid != INVALID_JOBID) {
        Kill(jobs[jobid].pid, SIGSTOP);
        jobs[jobid].status = STOPPED;
        printf("\n");
    }
}

void handler_sigchld (int sig) {
    pid_t pid;

    while ((pid = waitpid(-1, NULL, WNOHANG)) > 0) {
        jobid_t jobid = jobs_find_by_pid(pid);
        if (jobid == INVALID_JOBID)
            printf("Got a SIGCHLD for child with pid %d, but no corresponding job found\n", pid);
        else
            jobs[jobid].status = DONE;
    }
}

void shell_signals () {
    Signal(SIGCHLD, handler_sigchld);
    Signal(SIGINT, handler_sigint);
    Signal(SIGTSTP, handler_sigtstp);
}

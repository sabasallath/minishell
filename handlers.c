#include "myshell.h"
#include "jobs.h"
#include "handlers.h"

void handler_sigint(int sig) {
    int jobid = get_fg();
    if (jobid != -1) {
        Kill(jobs[jobid].pid, SIGINT);
        printf("\n");
    }
}

void handler_sigtstp(int sig) {
    int jobid = get_fg();
    if (jobid != -1) {
        Kill(jobs[jobid].pid, SIGSTOP);
        jobs[jobid].status = STOPPED;
        printf("\n");
    }
}

void handler_sigchld(int sig) {
    pid_t pid;

    while ((pid = waitpid(-1, NULL, WNOHANG)) > 0) {
        int jobid = get_job_by_pid(pid);
        if (jobid == -1)
            printf("Got a SIGCHLD for child with pid %d, but no corresponding job found\n", pid);
        else
            jobs[jobid].status = DONE;
    }
}

void shell_signals() {
    Signal(SIGCHLD, handler_sigchld);
    Signal(SIGINT, handler_sigint);
    Signal(SIGTSTP, handler_sigtstp);
}

/* eval : interprete une ligne de commande passee en parametre       */
#include <signal.h>
#include "jobs.h"
#include "minishell.h"
#include "exit.h"

#define Lock_jobs()                                 \
    sigset_t mask;                                  \
    sigset_t saved_mask;                            \
    do {                                            \
    sigemptyset(&mask);                             \
    sigaddset(&mask, SIGCHLD);                      \
    sigaddset(&mask, SIGINT);                       \
    sigaddset(&mask, SIGTSTP);                      \
    sigprocmask(SIG_SETMASK, &mask, &saved_mask);   \
} while (0)

#define Unlock_jobs() do {                          \
    sigprocmask(SIG_UNBLOCK, &saved_mask, NULL);    \
} while (0)

// fonctions externes
int parseline(char *buf, char **argv);
bool builtin_command(char **argv);
void replace_kill_jobs (char* buf, char** argv);

void exec_command(char** argv) {
    setpgid(0, 0);
    if (execvp(argv[0], argv) < 0) {
        printf("%s: Command not found.\n", argv[0]);
        exit(127);
    }
}

void eval(char *cmdline) {
    char *argv[MAXARGS];    // argv pour execve()
    char buf[MAXLINE];      // contient ligne commande modifiee
    char buf2[MAXLINE];     // pour les substitutions de jobid
                            // par pid pour la commande kill

    strcpy(buf, cmdline);
    bool bg = parseline(buf, argv);

    if (!builtin_command(argv)) {
        replace_kill_jobs(buf2, argv);

        int pid;
        if ((pid = Fork()) == 0) {
            exec_command(argv);
            // Ne retourne jamais
        }

        Lock_jobs();
        jobid_t jobid = jobs_add(pid, cmdline);
        Unlock_jobs();
        if (jobid == INVALID_JOBID) {
            printf("Error while trying to register job with pid %d (Maximum number of jobs (%d) reached ?)\n", pid, MAXJOBS);
        }

        if (bg)
            job_print_with_pid(jobid);
        else
            job_fg_wait(jobid);
    }

    exit_forget_next_forced();
    jobs_update();
    //unprotected
}
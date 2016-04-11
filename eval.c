/* eval : interprete une ligne de commande passee en parametre */
#include "jobs.h"
#include "minishell.h"
#include "exit.h"

// fonctions externes
bool builtin_command(char **argv);
bool parseline(char *buf, char **argv);
bool replace_kill_jobs (char* buf, char** argv);

void exec_command(char** argv) {
    Signal(SIGCHLD, SIG_DFL);
    Signal(SIGINT,  SIG_DFL);
    Signal(SIGTSTP, SIG_DFL);
    Signal(SIGTTOU, SIG_DFL);
    Signal(SIGTTIN, SIG_DFL);

    setpgid(0, 0);
    execvp(argv[0], argv);
    // Ne retourne que s'il y a un problème à l'éxecution de la
    // commande
    printf("%s: Command not found.\n", argv[0]);
    exit(127);
}

void eval(char *cmdline) {
    char *argv[MAXARGS];    // argv pour exec
    char buf[MAXLINE];      // contient ligne commande modifiee
    char buf2[MAXLINE];     // pour les substitutions de jobid
                            // par pid pour la commande kill
    strcpy(buf, cmdline);
    bool bg = parseline(buf, argv);

    if (!builtin_command(argv) && replace_kill_jobs(buf2, argv)) {
        int pid;
        if ((pid = Fork()) == 0) {
            exec_command(argv); // Ne retourne jamais
        }

        jobid_t jobid = jobs_add(pid, cmdline);
        if (jobid == INVALID_JOBID) {
            printf("Error while trying to register job with pid %d ", pid);
            printf("Maximum number of jobs (%d) reached ?)\n", MAXJOBS);
        }

        if (bg) {
            job_print_with_pid(jobid);
        }
        else {
            job_fg_wait(jobid, false);
        }
    }

    jobs_print_update();
    exit_forget_next_forced();
}
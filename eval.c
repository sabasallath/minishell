/* eval : interprete une ligne de commande passee en parametre       */
#include "jobs.h"
#include "minishell.h"
#include "exit.h"

// fonctions externes
int parseline(char *buf, char **argv);
bool builtin_command(char **argv);

void exec_command(char** argv) {
    setpgid(0, 0);
    if (execvp(argv[0], argv) < 0) {
        printf("%s: Command not found.\n", argv[0]);
        exit(127);
    }
}

void eval(char *cmdline) {
    char *argv[MAXARGS]; // argv pour execve()
    char buf[MAXLINE];   // contient ligne commande modifiee

    strcpy(buf, cmdline);
    bool bg = parseline(buf, argv);

    if (!builtin_command(argv)) {
        int pid;
        if ((pid = Fork()) == 0) {
            exec_command(argv);
            // Ne retourne jamais
        }

        jobid_t jobid = jobs_add(pid, cmdline);
        if (jobid == INVALID_JOBID) {
            printf("Error while trying to register job with pid %d (Maximum number of jobs (%d) reached ?)\n", pid, MAXJOBS);
        }

        if (bg)
            job_print_with_pid(jobid);
        else
            job_fg(jobid);
    }

    exit_forget_next_forced();
    jobs_update();
}

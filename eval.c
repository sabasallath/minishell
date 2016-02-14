/* eval : interprete une ligne de commande passee en parametre       */
#include "jobs.h"
#include "minishell.h"
#include "builtin.h"
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
            fg_wait(jobid);
    }

    exit_forget_next_forced();
    jobs_update();
}

// si le premier parametre est une commande integree,
// l'executer et renvoyer "vrai"
bool builtin_command(char **argv) {
    if (argv[0] == NULL)       // commande vide
        return true;
    if (!strcmp(argv[0], "&")) // ignorer & tout seul
        return true;

    if (!strcmp(argv[0], "exit") || !strcmp(argv[0], "quit")) {
        exit_try();
        return true;
    }

    if (!strcmp(argv[0], "jobs")) {
        jobs_print(STOPPED | BG);
        return true;
    }

    if (!strcmp(argv[0], "fg")) {
        jobid_t jobid = read_jobid(argv, STOPPED | BG);
        if (jobid != INVALID_JOBID)
            fg(jobid);
        return true;
    }

    if (!strcmp(argv[0], "bg")) {
        jobid_t jobid = read_jobid(argv, STOPPED);
        if (jobid != INVALID_JOBID)
            bg(jobid);
        return true;
    }

    if (!strcmp(argv[0], "int")) {
        jobid_t jobid = read_jobid(argv, FG | BG);
        if (jobid != INVALID_JOBID)
            interrupt(jobid);
        return true;
    }

    if (!strcmp(argv[0], "term")) {
        jobid_t jobid = read_jobid(argv, FG | BG);
        if (jobid != INVALID_JOBID)
            term(jobid);
        return true;
    }

    if (!strcmp(argv[0], "stop")) {
        jobid_t jobid = read_jobid(argv, FG | BG);
        if (jobid != INVALID_JOBID)
            stop(jobid);
        return true;
    }

    if (!strcmp(argv[0], "wait")) {
        builtin_wait();
        return true;
    }

    return false; // ce n'est pas une commande integree
}

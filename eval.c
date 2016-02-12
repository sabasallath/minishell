/* eval : interprete une ligne de commande passee en parametre       */
#include "jobs.h"
#include "minishell.h"
#include "builtin.h"

// fonctions externes
int parseline(char *buf, char **argv);
int builtin_command(char **argv);

void try_exit() {
    jobid_t jobid = jobs_find_first_by_status(~FREE);
    if (jobid == -1) {
        exit(0);
    }

    printf("Can't exit while there's some jobs left\n");
}

void eval(char *cmdline) {
    char *argv[MAXARGS]; // argv pour execve()
    char buf[MAXLINE];   // contient ligne commande modifiee
    int bg;              // arriere-plan ou premier plan ?
    pid_t pid;           // process id

    strcpy(buf, cmdline);
    bg = parseline(buf, argv);

    if (!builtin_command(argv)) {    // commande integree ?
        // si oui, executee directement
        if ((pid = Fork()) == 0) {   // si non, executee par un fils
            setpgid(0, 0);
            if (execvp(argv[0], argv) < 0) {
                printf("%s: Command not found.\n", argv[0]);
                exit(0);
            }
        }

        jobid_t jobid = jobs_add(pid, cmdline);
        if (!bg)
            fg(jobid);
        else
            printf("[%d] %d %s", jobid, pid, cmdline);
    }

    jobs_handle_done();
}

// si le premier parametre est une commande integree,
// l'executer et renvoyer "vrai"
int builtin_command(char **argv) {
    if (argv[0] == NULL)       // commande vide
        return 1;
    if (!strcmp(argv[0], "&")) // ignorer & tout seul
        return 1;

    if (!strcmp(argv[0], "exit")) { // commande "exit"
        try_exit();
        return 1;
    }
    if (!strcmp(argv[0], "quit")) { // commande "quitter"
        try_exit();
        return 1;
    }

    if (!strcmp(argv[0], "jobs")) {
        jobs_print(STOPPED | BG);
        return 1;
    }

    if (!strcmp(argv[0], "fg")) {
        jobid_t jobid = read_jobid(argv, STOPPED | BG);
        if (jobid != INVALID_JOBID)
            fg(jobid);
        return 1;
    }

    if (!strcmp(argv[0], "bg")) {
        jobid_t jobid = read_jobid(argv, STOPPED);
        if (jobid != INVALID_JOBID)
            bg(jobid);
        return 1;
    }

    if (!strcmp(argv[0], "stop")) {
        jobid_t jobid = read_jobid(argv, FG | BG);
        if (jobid != INVALID_JOBID)
            stop(jobid);
        return 1;
    }

    return 0; // ce n'est pas une commande integree
}

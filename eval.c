/* eval : interprete une ligne de commande passee en parametre       */
#include "jobs.h"
#include "myshell.h"
#include "builtin.h"

// fonctions externes
int parseline(char *buf, char **argv);
int builtin_command(char **argv);

int eval(char *cmdline) {
    char *argv[MAXARGS]; // argv pour execve()
    char buf[MAXLINE];   // contient ligne commande modifiee
    int bg;              // arriere-plan ou premier plan ?
    pid_t pid;           // process id

    strcpy(buf, cmdline);
    bg = parseline(buf, argv);

    if (!strcmp(argv[0], "exit")) // commande "exit"
        return 0;
    if (!strcmp(argv[0], "quit")) // commande "quitter"
        return 0;

    if (!builtin_command(argv)) {    // commande integree ?
        // si oui, executee directement
        if ((pid = Fork()) == 0) {   // si non, executee par un fils
            setpgid(0, 0);
            if (execvp(argv[0], argv) < 0) {
                printf("%s: Command not found.\n", argv[0]);
                exit(0);
            }
        }

        int jobid = add_new_job(pid, cmdline);
        if (!bg) // le pere attend fin du travail de premier plan
            fg(jobid);
        else       // travail d'arriere-plan, on imprime le pid
            printf("[%d] %d %s", jobid, pid, cmdline);
    }

    handle_done();

    return 1;
}

// si le premier parametre est une commande integree,
// l'executer et renvoyer "vrai"
int builtin_command(char **argv) {

    if (argv[0] == NULL) // commande vide
        return 1;
    if (!strcmp(argv[0], "&")) // ignorer & tout seul
        return 1;


    if (!strcmp(argv[0], "jobs")) {
        print_jobs();
        return 1;
    }

    if (!strcmp(argv[0], "fg")) {
        int jobid = read_jobid(argv, STOPPED | BG);
        if (jobid != -1)
            fg(jobid);
        return 1;
    }

    if (!strcmp(argv[0], "bg")) {
        int jobid = read_jobid(argv, STOPPED);
        if (jobid != -1)
            bg(jobid);
        return 1;
    }

    if (!strcmp(argv[0], "stop")) {
        int jobid = read_jobid(argv, FG | BG);
        if (jobid != -1)
            stop(jobid);
        return 1;
    }

    return 0;                     // ce n'est pas une commande integree
}

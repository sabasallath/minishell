/* eval : interprete une ligne de commande passee en parametre       */
#include "jobs.h"
#include "myshell.h"
#include "builtin.h"

// fonctions externes
int parseline(char *buf, char **argv);
int builtin_command(char **argv);

void eval(char *cmdline)
{
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

        int jobid = add_new_job(pid, cmdline);
        if (!bg) // le pere attend fin du travail de premier plan
            fg(jobid);
        else       // travail d'arriere-plan, on imprime le pid
            printf("[%d] %d %s", jobid, pid, cmdline);
    }

    handle_done();

    return;
}

void run_fg(char** argv) {
    int jobid;
    if (argv[1] == NULL) {
        jobid = default_fg_jobid();
        if (jobid == -1) {
            fprintf(stderr, "No background job");
            return;
        }
    }
    else {
        jobid = read_jobid(argv[1]);
        if (jobid == -1) {
            fprintf(stderr, "Wrong jobid: %s, expected number between 0 and 9", argv[1]);
            return;
        }
    }

    fg(jobid);
}

void run_bg(char** argv) {
    printf("bg not implemented yet !");
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
        run_fg(argv);
        return 1;
    }

    if (!strcmp(argv[0], "bg")) {
        run_bg(argv);
        return 1;
    }

    if (!strcmp(argv[0], "exit")) // commande "exit"
        exit(0);
    if (!strcmp(argv[0], "quit")) // commande "quitter"
        exit(0);

    return 0;                     // ce n'est pas une commande integree
}

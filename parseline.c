/* parseline - analyse ligne de commande, construit tableau argv[] */
#include "minishell.h"
#include "jobs.h"

bool parseline(char *buf, char **argv) {
    char *delim;         // pointe vers premier delimiteur espace
    int argc;            // nb d'arguments
    bool bg;             // travail d'arriere-plan ?

    buf[strlen(buf)-1] = ' ';     // remplacer '\n' final par espace
    while (*buf && (*buf == ' ')) // ignorer espaces au debut
        buf++;

    argc = 0;
    while ((delim = strchr(buf, ' '))) {   // construire liste args
        argv[argc++] = buf;
        *delim = '\0';
        buf = delim + 1;
        while (*buf && (*buf == ' '))      // ignorer espaces
            buf++;
    }
    argv[argc] = NULL;                     // termine liste d'args

    if (argc == 0)                         // ignorer ligne vide
        return true;

    if ((bg = (*argv[argc-1] == '&')) != 0) // travail d'arriere-plan ?
        argv[--argc] = NULL; // enleve le '&' dans la ligne de commande stockee

    return bg;               // 1 si travail d'arriere-plan, 0 sinon
}

// Defini dans builtin.c
jobid_t parse_jobid (char* arg);

pid_t read_job_pid(char* buf) {
    jobid_t jobid = parse_jobid(buf);
    return jobid == INVALID_JOBID
            ? 0
            : job_pid(jobid);
}

bool replace_kill_jobs (char* buf, char** argv) {
    if (strcmp(argv[0], "kill") != 0) return true;

    int i;
    for (i = 1; argv[i] != NULL; i++) {
        if (argv[i][0] == '%') {
            pid_t pid = read_job_pid(argv[i] + 1);
            if (pid == 0) {
                printf("Invalid jobid `%s`\n", argv[i] + 1);
                return false;
            }

            sprintf(buf, "%d", pid);
            argv[i] = buf;
            buf = strchr(buf, '\0') + 1;
        }
    }

    return true;
}
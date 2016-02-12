#include "jobs.h"
#include "minishell.h"
#include "handlers.h"

// fonctions externes
int eval(char*cmdline);
int parseline(char *buf, char **argv);
int builtin_command(char **argv);

int main() {
    char cmdline[MAXLINE];                 // ligne de commande

    jobs_init();
	shell_signals();

    while (1) {                            // boucle d'interpretation
        printf("<minishell> ");            // message d'invite
        Fgets(cmdline, MAXLINE, stdin);    // lire commande
        int try_exit = feof(stdin)         // fin (control-D)
            || !eval(cmdline);             // interpreter commande

        if (try_exit) {
            jobid_t jobid = jobs_find_first_by_status(~FREE);
            if (jobid == -1) {
                exit(0);
            }

            printf("Can't exit while there's some jobs left\n");
        }
    }
}

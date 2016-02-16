#include "jobs.h"
#include "minishell.h"
#include "exit.h"
#include "signals.h"

// fonctions externes
void force_exit();
void eval(char*cmdline);
int parseline(char *buf, char **argv);
int builtin_command(char **argv);

int main() {
    char cmdline[MAXLINE];                 // ligne de commande

    jobs_init();
	signals_init();

    while (1) {                            // boucle d'interpretation
        printf("<minishell> ");            // message d'invite
        Fgets(cmdline, MAXLINE, stdin);    // lire commande
        if (feof(stdin)) {                 // fin (control-D)
            printf("\n");
            exit_force();
        }
        else
            eval(cmdline);                 // interpreter commande
    }
}

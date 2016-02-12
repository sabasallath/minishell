#include "jobs.h"
#include "minishell.h"
#include "handlers.h"

// fonctions externes
void try_exit();
void eval(char*cmdline);
int parseline(char *buf, char **argv);
int builtin_command(char **argv);

int main() {
    char cmdline[MAXLINE];                 // ligne de commande

    jobs_init();
	shell_signals();

    while (1) {                            // boucle d'interpretation
        printf("<minishell> ");            // message d'invite
        Fgets(cmdline, MAXLINE, stdin);    // lire commande
        if (feof(stdin))                   // fin (control-D)
            try_exit();

        eval(cmdline);                     // interpreter commande
    }
}

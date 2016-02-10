#include "jobs.h"
#include "myshell.h"
#include "handlers.h"

// fonctions externes
void eval(char*cmdline);
int parseline(char *buf, char **argv);
int builtin_command(char **argv);

int main() {
    char cmdline[MAXLINE];                 // ligne de commande

    init_jobs();
	shell_signals();

    while (1) {                            // boucle d'interpretation
        printf("<my_shell> ");             // message d'invite
        Fgets(cmdline, MAXLINE, stdin);    // lire commande
        if (feof(stdin))                   // fin (control-D)
            exit(0);
        eval(cmdline);                     // interpreter commande
    }
}

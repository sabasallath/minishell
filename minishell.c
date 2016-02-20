#include "jobs.h"
#include "minishell.h"
#include "exit.h"
#include "signals.h"

// fonctions externes
void eval(char*cmdline);

int main() {
    char cmdline[MAXLINE];                 // ligne de commande

    stdin_is_tty = isatty(fileno(stdin));
    jobs_init();
	signals_init();

    while (1) {                            // boucle d'interpretation
        tty_printf("<minishell> ");        // message d'invite
        Fgets(cmdline, MAXLINE, stdin);    // lire commande
        if (feof(stdin)) {                 // fin (control-D)
            tty_printf("\n");
            exit_force();
        }
        else
            eval(cmdline);                 // interpreter commande
    }
}

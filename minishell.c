#include "jobs.h"
#include "minishell.h"
#include "exit.h"
#include "signals.h"
#include "dirs.h"
#include "terminal.h"

#include <readline/readline.h>
#include <readline/history.h>

// fonctions externes
void eval(char*cmdline);

int main() {
    char* cmdline; // ligne de commande

    terminal_init();
    cmdline = is_terminal ? NULL : malloc(MAXLINE * sizeof(char));
    jobs_init();
	signals_init();
    dirs_init();

    if (is_terminal) {
        // En console, utilisation de readline
        while (1) { // boucle d'interpretation
            cmdline = readline("<minishell> "); // lire commande

            if (cmdline == NULL) { // fin (control-D)
                free(cmdline);
                terminal_printf("\n");
                exit_force();
            }

            eval(cmdline); // interpreter commande
            add_history(cmdline);
            free(cmdline);
        }
    }
    else {
        // Sans console, lecture ligne à ligne bête et méchante
        while (1) { // boucle d'interpretation
            Fgets(cmdline, MAXLINE, stdin); // lire commande
            cmdline[strlen(cmdline) - 1] = '\0';

            if (feof(stdin)) { // fin (control-D)
                free(cmdline);
                terminal_printf("\n");
                exit_force();
            }

            eval(cmdline); // interpreter commande
        }
    }
}

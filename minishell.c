#include "jobs.h"
#include "minishell.h"
#include "exit.h"
#include "dirs.h"
#include "terminal.h"

// fonctions externes
void eval(char*cmdline);

int main() {
    char cmdline[MAXLINE];                 // ligne de commande

    terminal_init();
    jobs_init();
    dirs_init();

    while (1) {                            // boucle d'interpretation
        terminal_printf("<minishell> ");   // message d'invite
        Fgets(cmdline, MAXLINE, stdin);    // lire commande
        if (feof(stdin)) {                 // fin (control-D)
            terminal_printf("\n");
            exit_force();
        }
        else {
            eval(cmdline);                 // interpreter commande
        }
    }
}

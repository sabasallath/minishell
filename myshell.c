#include "jobs.h"
#include "myshell.h"

void handler_sigint(int sig) {
	int jobid = get_fg();
	if (jobid > -1) {
		Kill(jobs[jobid].pid, SIGINT);
	}
	printf("\n");
}

void handler_sigtstp(int sig) {
	int jobid = get_fg();
	if (jobid > -1) {
		jobs[jobid].status = STOPPED;
		Kill(jobs[jobid].pid, SIGSTOP);
	}
}

// fonctions externes
void eval(char*cmdline);
int parseline(char *buf, char **argv);
int builtin_command(char **argv);

int main() {
    char cmdline[MAXLINE];                 // ligne de commande

	Signal(SIGCHLD, handler_sigchld);
	Signal(SIGINT, handler_sigint);
	Signal(SIGTSTP, handler_sigtstp);

    while (1) {                            // boucle d'interpretation
        printf("<my_shell> ");             // message d'invite
        Fgets(cmdline, MAXLINE, stdin);    // lire commande
        if (feof(stdin))                   // fin (control-D)
            exit(0);
        eval(cmdline);                     // interpreter commande
    }
}

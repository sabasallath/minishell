/* eval : interprete une ligne de commande passee en parametre       */
#include <signal.h>
#include "jobs.h"
#include "minishell.h"
#include "exit.h"

// fonctions externes
int parseline(char *buf, char **argv);
bool builtin_command(char **argv);
void replace_kill_jobs (char* buf, char** argv);

void exec_command(char** argv) {
    setpgid(0, 0);
    if (execvp(argv[0], argv) < 0) {
        printf("%s: Command not found.\n", argv[0]);
        exit(127);
    }
}

void eval(char *cmdline) {
    char *argv[MAXARGS];    // argv pour execve()
    char buf[MAXLINE];      // contient ligne commande modifiee
    char buf2[MAXLINE];     // pour les substitutions de jobid
                            // par pid pour la commande kill
    Sigmask sigmask;

    strcpy(buf, cmdline);
    bool bg = parseline(buf, argv);

    if (!builtin_command(argv)) {
        interrupt_lock(sigmask);
        replace_kill_jobs(buf2, argv);

        int pid;
        if ((pid = Fork()) == 0) {
            interrupt_unlock(sigmask);
            exec_command(argv); // Ne retourne jamais
        }

        jobid_t jobid = jobs_add(pid, cmdline);
        if (jobid == INVALID_JOBID) {
            printf("Error while trying to register job with pid %d (Maximum number of jobs (%d) reached ?)\n", pid, MAXJOBS);
        }

        if (bg) {
             job_print_with_pid(jobid);
        }
        else {
            interrupt_unlock(sigmask);
            job_fg_wait(jobid);
            interrupt_lock(sigmask);
        }
    }
 
    exit_forget_next_forced();
    jobs_update();
    
    interrupt_unlock(sigmask);
}
#include "csapp.h"
#include "jobs.h"
#include "dirs.h"
#include "exit.h"

bool builtin_waiting = false;

jobid_t parse_jobid (char* arg) {
    if (*arg == '\0') {
        return INVALID_JOBID;
    }

	char* end = arg;
    jobid_t jobid = strtol(arg, &end, 10);
    if (*end != '\0') {
        // L'argument ne contient pas que des digits
        return INVALID_JOBID;
    }

    if (jobid <= 0 || jobid > MAXJOBS) {
    	return INVALID_JOBID;
    }

    return jobid - 1;
}

pid_t parse_pid(char* arg) {
    if (*arg == '\0') {
        return 0;
    }

    char* end = arg;
    pid_t pid = strtol(arg, &end, 10);
    return *end != '\0'
            ? 0 // L'argument ne contient pas que des digits
            : pid;
}

// Essaie de lire l'argument 1 de `argv` en tant que jobid
// Verifie la correspondance entre le status du job et le masque `status`
// Si l'argument est vide, choisi un job par défaut en accordance avec `status`
//
// Retourne le jobid en question
// ou INVALID_JOBID si aucun job adéquate n'a été trouvé
jobid_t read_jobid (char** argv, JobStatus status) {
    if (argv[1] == NULL) { // Pas d'argument spécifié pour la commande
        // Cherche un jobid par défaut.
        jobid_t jobid = jobs_find_first_by_status(status);
        if (jobid == INVALID_JOBID) {
            fprintf(stderr, "No default job found for command `%s`\n", argv[0]);
            return INVALID_JOBID;
        }

        return jobid;
    }

    jobid_t jobid;
    if (argv[1][0] == '%') {
        // L'argument est un id de job
        jobid = parse_jobid(argv[1] + 1);
        if (jobid == INVALID_JOBID) {
            fprintf(stderr, "Wrong jobid `%s`, expected number between 1 and %d\n",
                    argv[1] + 1, MAXJOBS);
            return INVALID_JOBID;
        }

        if (job_status_match(jobid, FREE)) {
            fprintf(stderr, "No job with id `%d`\n", jobid + 1);
            return INVALID_JOBID;
        }
    }
    else {
        // L'argument est un id de processus
        pid_t pid = parse_pid(argv[1]);
        if (pid == 0) {
            fprintf(stderr, "Invalid pid `%s`\n", argv[1]);
            return INVALID_JOBID;
        }

        jobid = jobs_find_by_pid(pid);
        if (jobid == INVALID_JOBID) {
            fprintf(stderr, "No job found for pid `%d`\n", pid);
            return INVALID_JOBID;
        }
    }

    if (!job_status_match(jobid, status)) {
        fprintf(stderr, "Can't use command `%s` for job with status `%s`\n",
                argv[0], job_status_str(jobid));
        return INVALID_JOBID;
    }

    return jobid;
}

void builtin_jobs () {
    jobs_print(STOPPED | RUNNING, false);
}

void builtin_fg (char** argv) {
    jobid_t jobid = read_jobid(argv, STOPPED | RUNNING);
    if (jobid == INVALID_JOBID)
        return;

    job_fg_wait(jobid, true);
}

void builtin_bg (char** argv) {
    jobid_t jobid = read_jobid(argv, STOPPED);
    if (jobid != INVALID_JOBID)
        job_change_status(jobid, SIGCONT);
}

void builtin_int (char** argv) {
    jobid_t jobid = read_jobid(argv, STOPPED | RUNNING);
    if (jobid != INVALID_JOBID)
        job_change_status(jobid, SIGINT);
}

void builtin_stop (char** argv) {
    jobid_t jobid = read_jobid(argv, RUNNING);
    if (jobid != INVALID_JOBID)
        job_change_status(jobid, SIGSTOP);
}

void builtin_term (char** argv) {
    jobid_t jobid = read_jobid(argv, STOPPED | RUNNING);
    if (jobid != INVALID_JOBID)
        job_change_status(jobid, SIGTERM);
}

void builtin_wait () {
    jobid_t jobid;
    builtin_waiting = true;
    while (builtin_waiting && (jobid = jobs_find_first_by_status(~(FREE | DONE | STOPPED))) != INVALID_JOBID) {
        sleep(0);
        jobs_print_update();
    }

    builtin_waiting = false;
}

#define builtin(name, exec) if (strcmp(argv[0], name) == 0) { exec; return true; }
// si le premier parametre est une commande integree,
// l'executer et renvoyer "vrai"
bool builtin_command(char **argv) {
    if (argv[0] == NULL)       // commande vide
        return true;
    if (strcmp(argv[0], "&") == 0) // ignorer & tout seul
        return true;

    builtin("exit", exit_try());
    builtin("quit", exit_try());

    builtin("jobs", builtin_jobs());
    builtin("fg",   builtin_fg(argv));
    builtin("bg",   builtin_bg(argv));
    builtin("int",  builtin_int(argv));
    builtin("term", builtin_term(argv));
    builtin("stop", builtin_stop(argv));
    builtin("wait", builtin_wait());

    builtin("dirs", dirs_print());
    builtin("cd", dirs_cd(argv[1]));
    builtin("pushd", dirs_pushd(argv[1]));
    builtin("popd", dirs_popd());

    return false; // ce n'est pas une commande integree
}

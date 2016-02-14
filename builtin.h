#ifndef __BUILTIN_H__
#define __BUILTIN_H__

#include "jobs.h"

// Essaie de lire l'argument 1 de `argv` en tant que jobid
// Verifie la correspondance entre le status du job et le masque `status`
// Si l'argument est vide, choisi un job par défaut en accordance avec `status`
//
// Retourne le jobid en question
// ou INVALID_JOBID si aucun job adéquate n'a été trouvé
jobid_t read_jobid (char** argv, JobStatus status);

// Envoi le signal `sig` de changement de status au job d'id `jobid`
// Attends le nombre de seconde `wait` (ou indéfiniment si `wait` = 0)
// indiqués pour valider le changement de status
void job_change_status(jobid_t jobid, int sig);

// Passe le job d'id `jobid` en premier plan.
// (En envoyant au préalable un signal SIGCONT a son processus si nécessaire)
void fg (jobid_t jobid);

// Passe le job d'id `jobid` en premier plan.
void fg_wait (jobid_t jobid);

// Envoi un signal SIGCONT au processus du job d'id `jobid`
void bg (jobid_t jobid);

// Envoi un signal SIGINT au processus du job d'id `jobid`
void interrupt (jobid_t jobid);

// Envoi un signal SIGSTOP au processus du job d'id `jobid`
void stop (jobid_t jobid);

// Envoi un signal SIGTERM au processus du job d'id `jobid`
void term (jobid_t jobid);

void builtin_wait ();

#endif
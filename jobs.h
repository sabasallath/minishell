#ifndef __JOBS_H__
#define __JOBS_H__

#include "minishell.h"

/////////////////////////////////////////////////////
// Types et constantes
/////////////////////////////////////////////////////

// Indique un id de job invalide.
#define INVALID_JOBID -1

// Nombre maximal de jobs
#define MAXJOBS 12

// Les jobid sont des valeurs parmis [0, MAXJOBS - 1]
// Ils sont incrementé lors des affichages vers
// l'interval [1, MAXJOBS]
typedef int8_t jobid_t;

// Les différents status sont codés par une position de bit chacun.
// L'interêt principal de ce codage est de pouvoir avoir des fonctions
// comme jobs_find_first_by_status(STOPPED | RUNNING)
//
// Pour un job donné, son statut ne peut qu'être l'un des suivants :
// FREE, STOPPED, RUNNING ou UPDATED
// Seul FG peut être combiné :
//    - Avec RUNNING, de manière évidente
//    - Avec UPDATED, pour indiquer qu'il n'est pas forcement
//      nécessaire d'afficher le changement de statut.
typedef enum {
    // L'id de job correspondant n'est pas utilisé
	FREE        = 1 << 0,
    // Le job est suspendu
	STOPPED     = 1 << 1,
    // Le job est en cours d'execution
	RUNNING     = 1 << 3,
    // Le job a reçu une mise à jour de statut et est en
    // attente de traitement
	UPDATED     = 1 << 4,
    // Le job est en premier plan
    FG          = 1 << 5
} JobStatus;

/////////////////////////////////////////////////////
// Fonctions de gestion des jobs
/////////////////////////////////////////////////////

// Initialise la gestion des jobs
void jobs_init ();

// Ajoute un nouveau job a la première place libre et retourne son jobid
// Retourne INVALID_JOBID si le nombre de job maximum a été atteint.
jobid_t jobs_add (pid_t pid, char* cmdline);

// Envoi le signal `sig` au processus du job d'id `jobid`
void job_kill(jobid_t jobid, int sig);

// Envoi le signal `sig` de changement de status au job d'id `jobid`
// Attends un certain temps pour permettre au changement de statut
// de se propager.
void job_change_status(jobid_t jobid, int sig);

// Passe le job d'id `jobid` en premier plan
void job_fg_wait(jobid_t jobid);

// Permet d'indiquer que le status du job `jobid`
// a changé. `update_status` correspond au résultat
// d'un appel aux fonctions `wait` ou `waitpid`
// Supposée être appelée depuis le handler de SIGCHLD
void job_updated(jobid_t jobid, int updated_status);

// Parcours l'ensemble des jobs de status UPDATED
// Réalise la mise à jour pour le job et affiche
// une ligne de description si pertinent
void jobs_update ();

// Réalise la mise à jour pour le job d'id `jobid`.
// et affiche une ligne de description.
void job_update (jobid_t jobid);

/////////////////////////////////////////////////////
// Fonctions utilitaires pour manipuler les jobs
/////////////////////////////////////////////////////

// Retourne le premier jobid correspondant au `status` donné.
// `status` peut être une combinaison bit à bit des différents status.
// Retourne INVALID_JOBID si non trouvé
jobid_t jobs_find_first_by_status (JobStatus status);

// Retourne le premier jobid correspondant au `pid` donné.
// Retourne INVALID_JOBID si non enregistré (ou libéré depuis)
jobid_t jobs_find_by_pid (pid_t pid);

// Indique si le status du job d'id `jobid` correspond à `status`,
// ce dernier pouvant être un masque combinaison de plusieurs status.
bool job_status_match(jobid_t jobid, JobStatus status);

// Affiche une ligne de description pour tous les jobs
// correspondant au masque `status`.
void jobs_print (JobStatus status);

// Retourne la chaine de caractère correspondant au
// status du job d'id `jobid`
char* job_status_str(jobid_t jobid);

// Affiche une ligne de description pour le job d'id `jobid`
void job_print (jobid_t jobid);

// Affiche une ligne de description pour le job d'id `jobid` avec son pid
void job_print_with_pid (jobid_t jobid);

// Affiche une ligne de description pour le job d'id `jobid`
// et le `status` donné
void job_print_with_status (jobid_t jobid, char* status);

#endif
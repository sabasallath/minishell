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
// FREE, STOPPED, RUNNING ou DONE
// Seul FG peut être combiné avec RUNNING
typedef enum {
    // L'id de job correspondant n'est pas utilisé
	FREE        = 1 << 0,
    // Le job est suspendu
	STOPPED     = 1 << 1,
    // Le job est en cours d'execution
	RUNNING     = 1 << 3,
    // Le job est terminé, il faut en notifier
    // l'utilisateur avant de le libérer
	DONE        = 1 << 4,
    // Le job est en premier plan
    FG          = 1 << 5
} JobStatus;

/////////////////////////////////////////////////////
// Fonctions de gestion des jobs
/////////////////////////////////////////////////////

// Initialise la gestion des jobs
void jobs_init ();

// Ajoute un nouveau job a la première place libre et retourne son jobid
// Retourne INVALID_JOBID si le nombre de job maximum a été atteint
jobid_t jobs_add (pid_t pid, char* cmdline);

// Envoi le signal `sig` au processus du job d'id `jobid`
// La valeur de retour est la même que celle de la fonction standard kill
int job_kill (jobid_t jobid, int sig);

// Envoi le signal `sig` de changement de status au job d'id `jobid`
// Attends un certain temps (best effort) pour permettre au changement
// de statut de se propager.
void job_change_status (jobid_t jobid, int sig);

// Passe le job d'id `jobid` en premier plan
void job_fg_wait (jobid_t jobid, bool print);

// Applique le changement de statut du job.
// `wait_status` correspond au résultat
// d'un appel aux fonctions `wait` ou `waitpid`
// Supposée être appelée depuis le handler de SIGCHLD
void job_update(jobid_t jobid, int wait_status);

// Parcours l'ensemble des jobs mis à jour
// Affiche une ligne de description pour chacun d'eux
// si besoin.
void jobs_print_update ();

// Réalise la mise à jour pour le job d'id `jobid`.
// Affiche une ligne de description si besoin.
void job_print_update (jobid_t jobid);

/////////////////////////////////////////////////////
// Fonctions utilitaires pour manipuler les jobs
/////////////////////////////////////////////////////

// Retourne le pid du job d'id `jobid`
pid_t job_pid (jobid_t jobid);

// Retourne le premier jobid correspondant au `status` donné.
// `status` peut être une combinaison bit à bit des différents status.
// Retourne INVALID_JOBID si non trouvé
jobid_t jobs_find_first_by_status (JobStatus status);

// Retourne le premier jobid correspondant au `pid` donné.
// Retourne INVALID_JOBID si non enregistré (ou libéré depuis)
jobid_t jobs_find_by_pid (pid_t pid);

// Indique si le status du job d'id `jobid` correspond à `status`,
// ce dernier pouvant être un masque combinaison de plusieurs status.
bool job_status_match (jobid_t jobid, JobStatus status);

// Affiche une ligne de description pour tous les jobs
// correspondant au masque `status`.
// Si updated est à false les jobs ayant reçu une mise à jour
// de status dont l'utilisateur n'a pas encore été notifié
// ne seront pas affichés
void jobs_print (JobStatus status, bool updated);

// Retourne la chaine de caractère correspondant au
// status du job d'id `jobid`
char* job_status_str (jobid_t jobid);

// Affiche une ligne de description pour le job d'id `jobid`
void job_print (jobid_t jobid);

// Affiche une ligne de description pour le job d'id `jobid` avec son pid
void job_print_with_pid (jobid_t jobid);

// Affiche une ligne de description pour le job d'id `jobid`
// et le `status` donné
void job_print_with_status (jobid_t jobid, char* status);

#endif
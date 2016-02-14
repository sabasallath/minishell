#ifndef __JOBS_H__
#define __JOBS_H__

#include "minishell.h"

typedef enum {
	FREE    = 1 << 0,
	STOPPED = 1 << 1,
	FG      = 1 << 2,
	BG      = 1 << 3,
	UPDATED = 1 << 4,
} JobStatus;

typedef int jobid_t;

// Indique un id de job invalide.
#define INVALID_JOBID -1

typedef struct {
	pid_t pid;
	JobStatus status;
	int argc;
	char cmdline[MAXLINE];
    int updated_status;
} Job;

// Nombre maximal de jobs
#define MAXJOBS 12

// Tableau statique contenant les jobs (libre ou non)
Job jobs[MAXJOBS];

// Initialise la gestion des jobs
void jobs_init ();

// Ajoute un nouveau job a la première place libre et retourne son jobid
// Retourne INVALID_JOBID si le nombre de job maximum a été atteint.
jobid_t jobs_add (pid_t pid, char* cmdline);

// Retourne le premier jobid correspondant au `status` donné.
// `status` peut être une combinaison bit à bit des différents status.
// Retourne INVALID_JOBID si non trouvé
jobid_t jobs_find_first_by_status (JobStatus status);

// Retourne le premier jobid correspondant au `pid` donné.
// Retourne INVALID_JOBID si non enregistré
jobid_t jobs_find_by_pid (pid_t pid);

// Affiche une ligne de description pour tous les jobs
// correspondant au masque `status`.
void jobs_print (JobStatus status);

// Parcours l'ensemble des jobs de status UPDATED
void jobs_update ();

// Réalise la mise à jour pour le job d'id `jobid`.
// et a une ligne de description.
void job_update (jobid_t jobid, bool print_ifexited);

// Retourne la chaine de caractère correspondant à `status`
char* job_status_str(jobid_t jobid);

// Indique si le status du job d'id `jobid` correspond à `status`,
// ce dernier pouvant être un masque combinaison de plusieurs status.
bool job_status_match(jobid_t jobid, JobStatus status);

// Affiche une ligne de description pour le job d'id `jobid`
void job_print (jobid_t jobid);

// Affiche une ligne de description pour le job d'id `jobid` avec son pid
void job_print_with_pid (jobid_t jobid);

// Affiche une ligne de description pour le job d'id `jobid`
// et le `status` donné
void job_print_with_status (jobid_t jobid, char* status);

#endif
#ifndef __BUILTIN_H__
#define __BUILTIN_H__

#include "jobs.h"

jobid_t read_jobid (char** argv, JobStatus status);
void fg (jobid_t jobid);
void bg (jobid_t jobid);
void interrupt (jobid_t jobid);
void stop (jobid_t jobid);
void term (jobid_t jobid);

#endif
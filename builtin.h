#ifndef __BUILTIN_H__
#define __BUILTIN_H__

#include "jobs.h"

int read_jobid (char** argv, JobStatus status);
void fg (int jobid);
void bg (int jobid);
void stop (int jobid);

#endif
#ifndef __BUILTIN_H__
#define __BUILTIN_H__

void fg (int jobid);
int read_jobid (char* argv);
int default_fg_jobid ();

#endif
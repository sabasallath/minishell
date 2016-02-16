#ifndef __PARSELINE_H__
#define __PARSELINE_H__

// parseline - analyse ligne de commande, construit tableau argv[]
bool parseline(char *buf, char **argv);
void replace_kill_jobs (char* buf, char** argv);

#endif
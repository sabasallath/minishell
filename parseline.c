/* parseline - analyse ligne de commande, construit tableau argv[] */
#include "myshell.h"

int parseline(char *buf, char **argv)
{
    char *delim;         // pointe vers premier delimiteur espace
    int argc;            // nb d'arguments
    int bg;              // travail d'arriere-plan ?

    buf[strlen(buf)-1] = ' ';     // remplacer '\n' final par espace
    while (*buf && (*buf == ' ')) // ignorer espaces au debut
        buf++;

    argc = 0;
    while ((delim = strchr(buf, ' '))) {   // construire liste args
        argv[argc++] = buf;
        *delim = '\0';
        buf = delim + 1;
        while (*buf && (*buf == ' '))      // ignorer espaces
            buf++;
    }
    argv[argc] = NULL;                     // termine liste d'args

    if (argc == 0)                         // ignorer ligne vide
        return 1;

    if ((bg = (*argv[argc-1] == '&')) != 0) // travail d'arriere-plan ?
        argv[--argc] = NULL; // enleve le '&' dans la ligne de commande stockee

    return bg;               // 1 si travail d'arriere-plan, 0 sinon
}

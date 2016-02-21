#include "minishell.h"
#include <sys/types.h>
#include <pwd.h>
#include "dirs.h"

#define MAX_PATH  500
#define MAX_STACK 20

char* stack[MAX_STACK];
int stack_index;

void dirs_init() {
    stack_index = 0;
    stack[stack_index] = getcwd(NULL, MAX_PATH);
}

void dirs_print() {
    for (int i = stack_index; i >= 0; i--) {
        printf("%s", stack[i]);
        if (i > 0) {
            printf(" ");
        }
    }
    printf("\n");
}

void dirs_cd(char* path) {
    if (path == NULL) {
        path = getpwuid(getuid())->pw_dir;
    }

    if (chdir(path) == 0)
        stack[stack_index] = getcwd(NULL, MAX_PATH);
    else
        fprintf(stderr, "[Error] cd %s : %s\n", path, strerror(errno));
}

void dirs_pushd(char* path) {
    if (path == NULL) {
        fprintf(stderr, "[Error] pushd: Missing path argument\n");
        return;
    }

    stack_index++;
    dirs_cd(path);
}

void dirs_popd() {
    if (stack_index == 0) {
        fprintf(stderr, "[Error] popd: Directory stack empty\n");
    }
    else {
        free(stack[stack_index]);
        stack_index--;
        dirs_cd(stack[stack_index]);
    }
}
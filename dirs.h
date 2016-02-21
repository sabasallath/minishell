#ifndef __DIRS_H__
#define __DIRS_H__

void dirs_init();
void dirs_print();
void dirs_cd(char* path);
void dirs_pushd(char* path);
void dirs_popd();

#endif
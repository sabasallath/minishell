cmake_minimum_required(VERSION 2.8)
project(minishell C)

set(CMAKE_C_COMPILER gcc)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Werror -pedantic -g -std=c99 -D_GNU_SOURCE")

set(minishell_SRC
    builtin.c
    csapp.c
    csapp.h
    dirs.c
    dirs.h
    eval.c
    exit.c
    exit.h
    jobs.c
    jobs.h
    minishell.c
    minishell.h
    parseline.c
    signals.c
    signals.h)

add_executable(minishell ${minishell_SRC})
target_link_libraries(minishell pthread)

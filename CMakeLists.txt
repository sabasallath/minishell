cmake_minimum_required(VERSION 2.8)
project(minishell C)

set(CMAKE_C_COMPILER gcc)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Werror -pedantic -g -std=c99")

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
    jobs_signals.c
    minishell.c
    minishell.h
    parseline.c
    terminal.c
    terminal.h)

add_executable(minishell ${minishell_SRC})
target_compile_options(minishell PUBLIC -D_GNU_SOURCE)
target_link_libraries(minishell pthread)

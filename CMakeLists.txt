project(minishell C)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Werror -pedantic -g -std=c99 -D_GNU_SOURCE")

set(SOURCE_FILES
    builtin.c
    csapp.c
    csapp.h
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

add_executable(minishell ${SOURCE_FILES})
target_link_libraries(minishell pthread)

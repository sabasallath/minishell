cmake_minimum_required(VERSION 2.8)
project(minishell C)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Werror -pedantic -g -std=c99 -D_GNU_SOURCE")

AUX_SOURCE_DIRECTORY("." SOURCE_FILES)

add_executable(minishell ${SOURCE_FILES})
target_link_libraries(minishell pthread)

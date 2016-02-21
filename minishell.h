#include "csapp.h"
#include <stdint.h>
#include <stdbool.h>
#include <termios.h>

#define MAXARGS 128

bool is_terminal;
#define tty_printf(...) if (is_terminal) printf(__VA_ARGS__)

#define terminal STDIN_FILENO
struct termios termios;
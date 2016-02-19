#include "csapp.h"
#include "stdint.h"
#include "stdbool.h"

#define MAXARGS 128

bool stdin_is_tty;
#define tty_printf(...) if (stdin_is_tty) printf(__VA_ARGS__)
#ifndef __TERMINAL_H__
#define __TERMINAL_H__

bool is_terminal;
#define terminal_printf(...) if (is_terminal) printf(__VA_ARGS__)

void terminal_init();
void terminal_init_termios(struct termios* termios);
void terminal_grab(pid_t pid, struct termios* termios);
void terminal_restore(struct termios* termios);

#endif
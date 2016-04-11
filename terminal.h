#ifndef __TERMINAL_H__
#define __TERMINAL_H__

typedef struct termios TerminalControl;

bool is_terminal;
#define terminal_printf(...) if (is_terminal) printf(__VA_ARGS__)

void terminal_init();
void terminal_init_control(TerminalControl* data);
void terminal_give_control(pid_t pid, TerminalControl* ctl);
void terminal_take_back_control(TerminalControl* ctl);

#endif
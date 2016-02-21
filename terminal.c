#include "minishell.h"
#include "terminal.h"

pid_t shell_pid;
struct termios shell_termios;

void terminal_init() {
    shell_pid = getpid();
    is_terminal = isatty(STDIN_FILENO);
    if (is_terminal) {
        setpgid(shell_pid, shell_pid);
        tcsetpgrp(STDIN_FILENO, shell_pid);
        tcgetattr(STDIN_FILENO, &shell_termios);

        Signal(SIGTTIN, SIG_IGN);
        Signal(SIGTTOU, SIG_IGN);
    }
}

void terminal_init_termios(struct termios* termios) {
    *termios = shell_termios;
}

void terminal_grab(pid_t pid, struct termios* termios) {
    if (is_terminal) {
        setpgid(pid, pid);
        tcgetattr(STDIN_FILENO, &shell_termios);
        tcsetpgrp(STDIN_FILENO, pid);
        tcsetattr(STDIN_FILENO, TCSADRAIN, termios);
    }
}

void terminal_restore(struct termios* termios) {
    if (is_terminal) {
        tcgetattr(STDIN_FILENO, termios);
        tcsetpgrp(STDIN_FILENO, shell_pid);
        tcsetattr(STDIN_FILENO, TCSADRAIN, &shell_termios);
    }
}

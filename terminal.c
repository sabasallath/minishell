#include "minishell.h"
#include "terminal.h"

pid_t shell_pid;
struct termios shell_ctl;

void terminal_init() {
    shell_pid = getpid();
    is_terminal = isatty(STDIN_FILENO);
    if (is_terminal) {
        setpgid(shell_pid, shell_pid);
        tcsetpgrp(STDIN_FILENO, shell_pid);
        tcgetattr(STDIN_FILENO, &shell_ctl);

        // On remplace les handlers par défaut de ces signaux
        // (arrêt du process) qui vont être lancés quand on
        // donne le contrôle du terminal aux jobs.
        Signal(SIGTTIN, SIG_IGN);
        Signal(SIGTTOU, SIG_IGN);
    }
}

void terminal_init_control(TerminalControl* ctl) {
    *ctl = shell_ctl;
}

void terminal_give_control(pid_t pid, TerminalControl* ctl) {
    if (is_terminal) {
        setpgid(pid, pid);
        tcgetattr(STDIN_FILENO, &shell_ctl);
        tcsetpgrp(STDIN_FILENO, pid);
        tcsetattr(STDIN_FILENO, TCSADRAIN, ctl);
    }
}

void terminal_take_back_control(TerminalControl* ctl) {
    if (is_terminal) {
        tcgetattr(STDIN_FILENO, ctl);
        tcsetpgrp(STDIN_FILENO, shell_pid);
        tcsetattr(STDIN_FILENO, TCSADRAIN, &shell_ctl);
    }
}

#define _XOPEN_SOURCE 600
#include "shell.h"
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <pty.h>
#include <utmp.h> // para login_tty
#include <sys/wait.h>
#include "logging.h"

int bgl_spawn_shell(int slave_fd, const char* shell_path) {
    pid_t pid = fork();
    if (pid == 0) {
        if (login_tty(slave_fd) < 0) {
            perror("login_tty");
            _exit(1);
        }
        execlp(shell_path, shell_path, (char*)NULL);
        perror("execlp");
        _exit(1);
    }
    bgl_log("Shell spawned: pid=%d shell=%s", pid, shell_path);
    return pid;
}

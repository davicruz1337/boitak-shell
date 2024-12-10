#define _XOPEN_SOURCE 600
#include "terminal.h"
#include "shell.h"
#include <stdio.h>
#include <unistd.h>
#include <pty.h>
#include <sys/ioctl.h>
#include <signal.h>
#include "logging.h"

static int g_master = -1;
static int g_shell_pid = -1;

int bgl_terminal_open_pty(const char* shell_path) {
    int master, slave;
    if (openpty(&master, &slave, NULL, NULL, NULL) == -1) {
        perror("openpty");
        return -1;
    }
    g_shell_pid = bgl_spawn_shell(slave, shell_path);
    close(slave);
    g_master = master;
    return g_master;
}

int bgl_terminal_shell_pid() {
    return g_shell_pid;
}

int bgl_terminal_master_fd() {
    return g_master;
}

void bgl_terminal_close() {
    if (g_master >= 0) close(g_master);
    g_master = -1;
}



void bgl_terminal_send_signal(int signo) {
    if (g_shell_pid>0) kill(g_shell_pid, signo);
}



void bgl_terminal_resize(int rows, int cols) {
    if (g_master < 0) return;
    struct winsize ws;
    ws.ws_row = (unsigned short)rows;
    ws.ws_col = (unsigned short)cols;
    ws.ws_xpixel = 0;
    ws.ws_ypixel = 0;
    ioctl(g_master, TIOCSWINSZ, &ws);
    if (g_shell_pid > 0) {
        kill(g_shell_pid, SIGWINCH);
    }
}
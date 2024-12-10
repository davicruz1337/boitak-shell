#ifndef TERMINAL_H
#define TERMINAL_H

int bgl_terminal_open_pty(const char* shell_path);
int bgl_terminal_shell_pid();
int bgl_terminal_master_fd();
void bgl_terminal_close();
void bgl_terminal_resize(int rows, int cols);
#endif

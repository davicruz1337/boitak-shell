#ifndef UI_H
#define UI_H

#include <stdbool.h>

void bgl_ui_show_ascii_art(bool show);
void bgl_ui_print_prompt(const char *user, bool root, const char *prompt_color, double last_cmd_time);
int bgl_ui_read_command(char* buf, int size, const char* user, bool root, const char* prompt_color, double last_cmd_time);
void bgl_ui_handle_output(int master_fd);

#endif

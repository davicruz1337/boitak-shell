#include "ui.h"
#include "utils.h"
#include "line_editor.h"
#include <stdio.h>
#include <string.h>
#include <unistd.h>

void bgl_ui_show_ascii_art(bool show) {
    if (!show) return;
    const char *art =
    "\x1b[36m\n"
    "  ____              _       _     \n"
    " |  _ \\            | |     | |    \n"
    " | |_) | ___   ___ | |_ ___| |__  \n"
    " |  _ < / _ \\ / _ \\| __/ __| '_ \\ \n"
    " | |_) | (_) | (_) | || (__| | | |\n"
    " |____/ \\___/ \\___/ \\__\\___|_| |_|\n"
    "\x1b[0m\n";
    write(STDOUT_FILENO, art, strlen(art));
}

void bgl_ui_print_prompt(const char *user, bool root, const char *prompt_color, double last_cmd_time) {
    char pwd[512];
    getcwd(pwd, sizeof(pwd));
    const char* col = bgl_color_code(prompt_color);
    const char* user_col = root ? "\x1b[31m" : "\x1b[32m";

    char timeinfo[64];
    if (last_cmd_time >= 0.0)
        snprintf(timeinfo, sizeof(timeinfo), "[%.3fs]", last_cmd_time);
    else
        timeinfo[0]='\0';

    // Nenhum espaço extra no fim, prompt no canto esquerdo
    char prompt[1024];
    if (timeinfo[0]) {
        snprintf(prompt, sizeof(prompt), "%s%s%s:%s%s%s %s>>\x1b[0m", 
                 user_col, user, "\x1b[0m",
                 col, pwd, "\x1b[0m",
                 timeinfo);
    } else {
        snprintf(prompt, sizeof(prompt), "%s%s%s:%s%s%s >>\x1b[0m", 
                 user_col, user, "\x1b[0m",
                 col, pwd, "\x1b[0m");
    }

    write(STDOUT_FILENO, prompt, strlen(prompt));
    write(STDOUT_FILENO, " ", 1); // um espaço após o prompt para separação do texto digitado
    fsync(STDOUT_FILENO);
}

int bgl_ui_read_command(char* buf, int size, const char* user, bool root, const char* prompt_color, double last_cmd_time) {
    (void)user; (void)root; (void)prompt_color; (void)last_cmd_time;
    // Apenas chama le_read_line, sem prompt, pois já impresso
    return le_read_line("", buf, size);
}

void bgl_ui_handle_output(int master_fd) {
    char buf[1024];
    ssize_t n = read(master_fd, buf, sizeof(buf));
    if (n > 0) {
        write(STDOUT_FILENO, buf, n);
    }
}

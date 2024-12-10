#define _XOPEN_SOURCE 600
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdbool.h>
#include <time.h>
#include <sys/wait.h>
#include <errno.h>
#include <ctype.h>

#include "ui.h"
#include "plugin.h"
#include "logging.h"
#include "utils.h"
#include "line_editor.h"

int main(int argc, char *argv[]) {
    (void)argc; (void)argv;

    bgl_log_init("bgl_terminal.log");
    bgl_log("Iniciando Boitak terminal...");

    bool ascii_art = true;
    const char* prompt_color = "green";
    char *u = getenv("USER");
    if (!u) u = "user";
    bool isroot = (geteuid() == 0);
    double last_cmd_time = -1.0;

    bgl_plugin_init();
    bgl_plugin_load_all("plugins");

    bgl_ui_show_ascii_art(ascii_art);
    le_init();

    char cmd[2048];

    for (;;) {
        bgl_ui_print_prompt(u, isroot, prompt_color, last_cmd_time);

        int r = bgl_ui_read_command(cmd, sizeof(cmd), u, isroot, prompt_color, last_cmd_time);
        if (r < 0) {
            // EOF ou erro na leitura
            break;
        }

        // Remove espaços iniciais e finais
        char *start = cmd;
        while (isspace((unsigned char)*start)) start++;
        char *end = start + strlen(start) - 1;
        while (end > start && isspace((unsigned char)*end)) *end-- = '\0';

        if (*start == '\0') {
            // comando vazio
            continue;
        }

        if (strcmp(start, "exit") == 0) {
            break;
        }

        bgl_plugin_notify_command(start);

        char line_copy[2048];
        strncpy(line_copy, start, sizeof(line_copy)-1);
        line_copy[sizeof(line_copy)-1] = '\0';

        char* tokens[128];
        int pos=0;
        char* tok = strtok(line_copy, " \t\r\n");
        while (tok && pos<127) {
            tokens[pos++]=tok;
            tok=strtok(NULL, " \t\r\n");
        }
        tokens[pos]=NULL;
        if (!tokens[0]) continue;

        struct timespec begin, finish;
        clock_gettime(CLOCK_MONOTONIC, &begin);

        pid_t pid = fork();
        if (pid == 0) {
            execvp(tokens[0], tokens);
            if (errno == ENOENT) {
                fprintf(stderr, "%s: Command not found\n", tokens[0]);
            } else {
                perror("execvp");
            }
            _exit(127);
        } else if (pid > 0) {
            int status;
            waitpid(pid, &status, 0);
        } else {
            perror("fork");
        }

        clock_gettime(CLOCK_MONOTONIC, &finish);
        double elapsed = (finish.tv_sec - begin.tv_sec) + (finish.tv_nsec - begin.tv_nsec)/1000000000.0;
        last_cmd_time = elapsed;
    }

    bgl_plugin_cleanup();
    le_deinit();
    bgl_log("Encerrando Boitak terminal.");
    bgl_log_close();
    return 0;
}

#ifndef PLUGIN_H
#define PLUGIN_H

#include <stddef.h>

typedef void (*plugin_modify_prompt_func)(char* promptbuf, size_t bufsize);
typedef void (*plugin_on_command_func)(const char* cmd);

void bgl_plugin_init();
void bgl_plugin_load_all(const char *dir);
void bgl_plugin_apply_prompt(char* promptbuf, size_t bufsize);
void bgl_plugin_notify_command(const char* cmd);
void bgl_plugin_cleanup();

#endif

#define _XOPEN_SOURCE 600
#include "plugin.h"
#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <dlfcn.h>
#include <string.h>
#include "logging.h"

typedef struct {
    void *handle;
    plugin_modify_prompt_func mod_prompt;
    plugin_on_command_func on_cmd;
} bgl_plugin_t;

static bgl_plugin_t **g_plugins = NULL;
static int g_num_plugins = 0;

void bgl_plugin_init() {
    g_plugins = NULL;
    g_num_plugins = 0;
}

void bgl_plugin_load_all(const char *dir) {
    DIR *d = opendir(dir);
    if (!d) {
        bgl_log("No plugins directory found: %s", dir);
        return;
    }
    struct dirent *ent;
    while ((ent = readdir(d)) != NULL) {
        if (strstr(ent->d_name, ".so")) {
            char path[1024];
            snprintf(path, sizeof(path), "%s/%s", dir, ent->d_name);
            void *h = dlopen(path, RTLD_NOW);
            if (!h) {
                bgl_log("Failed to load plugin %s: %s", path, dlerror());
                continue;
            }
            plugin_modify_prompt_func mp = dlsym(h, "plugin_modify_prompt");
            plugin_on_command_func oc = dlsym(h, "plugin_on_command");

            bgl_plugin_t *p = malloc(sizeof(bgl_plugin_t));
            p->handle = h;
            p->mod_prompt = mp;
            p->on_cmd = oc;

            g_plugins = realloc(g_plugins, sizeof(bgl_plugin_t*)*(g_num_plugins+1));
            g_plugins[g_num_plugins] = p;
            g_num_plugins++;

            bgl_log("Loaded plugin: %s", path);
        }
    }
    closedir(d);
}

void bgl_plugin_apply_prompt(char* promptbuf, size_t bufsize) {
    for (int i=0; i<g_num_plugins; i++) {
        if (g_plugins[i]->mod_prompt) {
            g_plugins[i]->mod_prompt(promptbuf, bufsize);
        }
    }
}

void bgl_plugin_notify_command(const char* cmd) {
    for (int i=0; i<g_num_plugins; i++) {
        if (g_plugins[i]->on_cmd) {
            g_plugins[i]->on_cmd(cmd);
        }
    }
}

void bgl_plugin_cleanup() {
    for (int i=0; i<g_num_plugins; i++) {
        dlclose(g_plugins[i]->handle);
        free(g_plugins[i]);
    }
    free(g_plugins);
    g_plugins = NULL;
    g_num_plugins = 0;
}

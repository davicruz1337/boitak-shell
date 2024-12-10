#define _XOPEN_SOURCE 600
#include "config.h"
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <strings.h>

static void trim(char *s) {
    char *end;
    while(isspace((unsigned char)*s)) s++;
    if(*s == 0) return;
    end = s + strlen(s) - 1;
    while(end > s && isspace((unsigned char)*end)) end--;
    end[1] = '\0';
    memmove(s, s, strlen(s)+1);
}

bool bgl_config_load(const char* filename, bgl_config_t* cfg) {
    FILE *f = fopen(filename, "r");
    // defaults
    strcpy(cfg->prompt_color, "green");
    cfg->ascii_art = true;
    cfg->plugins_dir[0] = '\0';
    cfg->env_prep_script[0] = '\0';
    cfg->custom_path[0] = '\0';
    cfg->python_env[0] = '\0';
    cfg->git_editor[0] = '\0';

    if (!f) return false;

    char line[512];
    while (fgets(line, sizeof(line), f)) {
        trim(line);
        if (line[0] == '#' || line[0] == 0) continue;
        char *eq = strchr(line, '=');
        if (!eq) continue;
        *eq = 0;
        char *key = line;
        char *val = eq+1;
        trim(key); trim(val);

        if (strcasecmp(key, "prompt_color")==0) {
            strncpy(cfg->prompt_color, val, sizeof(cfg->prompt_color)-1);
        } else if (strcasecmp(key, "ascii_art")==0) {
            cfg->ascii_art = (strcasecmp(val,"true")==0);
        } else if (strcasecmp(key, "plugins_dir")==0) {
            strncpy(cfg->plugins_dir, val, sizeof(cfg->plugins_dir)-1);
        } else if (strcasecmp(key, "env_prep_script")==0) {
            strncpy(cfg->env_prep_script, val, sizeof(cfg->env_prep_script)-1);
        } else if (strcasecmp(key, "custom_path")==0) {
            strncpy(cfg->custom_path, val, sizeof(cfg->custom_path)-1);
        } else if (strcasecmp(key, "python_env")==0) {
            strncpy(cfg->python_env, val, sizeof(cfg->python_env)-1);
        } else if (strcasecmp(key, "git_editor")==0) {
            strncpy(cfg->git_editor, val, sizeof(cfg->git_editor)-1);
        }
    }

    fclose(f);
    return true;
}

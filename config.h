#ifndef CONFIG_H
#define CONFIG_H

#include <stdbool.h>

typedef struct {
    char prompt_color[64];
    bool ascii_art;
    char plugins_dir[256];      // diretório de plugins
    char env_prep_script[256];  // script de preparação de ambiente
    char custom_path[1024];     // PATH customizado
    char python_env[1024];      // PYTHONPATH customizado
    char git_editor[256];       // Editor para git
} bgl_config_t;

bool bgl_config_load(const char* filename, bgl_config_t* cfg);

#endif

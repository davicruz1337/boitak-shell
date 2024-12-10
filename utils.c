#define _XOPEN_SOURCE 600
#include "utils.h"
#include <string.h>
#include <ctype.h>

const char* bgl_color_code(const char* name) {
    if (!name) return "\x1b[0m";
    if (strcasecmp(name,"red")==0) return "\x1b[31m";
    if (strcasecmp(name,"green")==0) return "\x1b[32m";
    if (strcasecmp(name,"yellow")==0) return "\x1b[33m";
    if (strcasecmp(name,"blue")==0) return "\x1b[34m";
    if (strcasecmp(name,"magenta")==0) return "\x1b[35m";
    if (strcasecmp(name,"cyan")==0) return "\x1b[36m";
    if (strcasecmp(name,"white")==0) return "\x1b[37m";
    if (strcasecmp(name,"gray")==0) return "\x1b[90m";
    return "\x1b[0m";
}

void bgl_str_trim(char *s) {
    char *end;
    while(isspace((unsigned char)*s)) s++;
    if(*s == 0) {
        return;
    }
    end = s + strlen(s) - 1;
    while(end > s && isspace((unsigned char)*end)) end--;
    end[1] = '\0';
    memmove(s, s, strlen(s)+1);
}

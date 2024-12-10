#define _XOPEN_SOURCE 600
#include "line_editor.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <termios.h>
#include <ctype.h>
#include <sys/ioctl.h>

static struct termios orig_termios;
static char **history = NULL;
static int history_count = 0;
static int history_pos = 0;

static int screen_cols = 80;
static int screen_rows = 24;

static int multiline = 1; // habilita multiline

// tab completion dummy commands
static const char* dummy_cmds[] = {
    "ls", "cd", "cat", "echo", "exit", "git", "make", "gcc", "bgl", "boitak",
    "python", "perl", "ruby", "cargo", "rustc", NULL
};

static void enable_raw_mode() {
    tcgetattr(STDIN_FILENO, &orig_termios);
    struct termios raw = orig_termios;
    raw.c_lflag &= ~(ICANON | ECHO | IEXTEN | ISIG);
    raw.c_iflag &= ~(IXON | ICRNL | BRKINT | INPCK | ISTRIP);
    raw.c_oflag &= ~(OPOST);
    raw.c_cflag |= (CS8);
    tcsetattr(STDIN_FILENO, TCSANOW, &raw);
}

static void disable_raw_mode() {
    tcsetattr(STDIN_FILENO, TCSANOW, &orig_termios);
}

void le_init() {
    enable_raw_mode();

    // descobrir tamanho terminal
    struct winsize ws;
    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws) == 0) {
        screen_cols = ws.ws_col;
        screen_rows = ws.ws_row;
    }
}

void le_deinit() {
    disable_raw_mode();
    for (int i=0;i<history_count;i++) {
        free(history[i]);
    }
    free(history);
    history=NULL;
    history_count=0;
}

// adiciona histórico
void le_add_history(const char* line) {
    if (line[0]==0) return; 
    history = realloc(history, sizeof(char*)*(history_count+1));
    history[history_count] = strdup(line);
    history_count++;
    history_pos = history_count;
}

// redesenha linha
static void le_clear_line() {
    // limpa linha inteira da esquerda
    write(STDOUT_FILENO, "\r", 1);
    const char *clearseq = "\x1b[0J"; // limpa do cursor p/ baixo
    write(STDOUT_FILENO, clearseq, strlen(clearseq));
}

static void le_move_cursor_start() {
    write(STDOUT_FILENO, "\r", 1);
}

static void le_reprint_prompt_and_buf(const char* prompt, const char* buf, int pos) {
    le_clear_line();
    // print prompt
    write(STDOUT_FILENO, prompt, strlen(prompt));

    // print buf
    int plen = (int)strlen(prompt);
    write(STDOUT_FILENO, buf, strlen(buf));

    // se multiline e buf grande, pode rolar wrap
    // mas simplificaremos apenas assumindo terminal faz wrap

    // reposiciona cursor
    // volta ao prompt inicio
    le_move_cursor_start();
    // move cursor right plen+pos
    char seq[64];
    int total = plen + pos;
    snprintf(seq, sizeof(seq), "\x1b[%dC", total);
    write(STDOUT_FILENO, seq, strlen(seq));
}

// move cursor para o inicio da linha (Ctrl+A)
static void le_cursor_start(char* buf, int *pos) {
    *pos = 0;
}

// move cursor para o fim da linha (Ctrl+E)
static void le_cursor_end(char* buf, int *pos) {
    *pos = (int)strlen(buf);
}

// deleta até o fim da linha (Ctrl+K)
static void le_kill_line_end(char* buf, int *pos) {
    buf[*pos] = '\0';
}

// deleta até o inicio da linha (Ctrl+U)
static void le_kill_line_start(char* buf, int *pos) {
    int len = (int)strlen(buf);
    memmove(buf, &buf[*pos], len - *pos +1);
    *pos = 0;
}

// apaga uma palavra antes do cursor (Ctrl+W)
static void le_kill_prev_word(char* buf, int *pos) {
    int start = *pos;
    while (start > 0 && isspace((unsigned char)buf[start-1])) start--;
    while (start > 0 && !isspace((unsigned char)buf[start-1])) start--;
    memmove(&buf[start], &buf[*pos], strlen(&buf[*pos])+1);
    *pos = start;
}

// avança uma palavra (Alt+F)
static void le_forward_word(char* buf, int *pos) {
    int len = (int)strlen(buf);
    int p = *pos;
    while (p < len && !isspace((unsigned char)buf[p])) p++;
    while (p < len && isspace((unsigned char)buf[p])) p++;
    *pos = p;
}

// retrocede uma palavra (Alt+B)
static void le_backward_word(char* buf, int *pos) {
    int p = *pos;
    while (p > 0 && isspace((unsigned char)buf[p-1])) p--;
    while (p > 0 && !isspace((unsigned char)buf[p-1])) p--;
    *pos = p;
}

// histórico para cima/baixo
static void le_history_up(char* buf, int bufsize, int *buflen, int *pos) {
    if (history_count>0 && history_pos>0) {
        history_pos--;
        strncpy(buf, history[history_pos], bufsize-1);
        buf[bufsize-1]=0;
        *buflen=(int)strlen(buf);
        *pos=*buflen;
    }
}

static void le_history_down(char* buf, int bufsize, int *buflen, int *pos) {
    if (history_pos<history_count) {
        history_pos++;
        if (history_pos==history_count) {
            buf[0]=0;
            *buflen=0;
            *pos=0;
        } else {
            strncpy(buf, history[history_pos], bufsize-1);
            buf[bufsize-1]=0;
            *buflen=(int)strlen(buf);
            *pos=*buflen;
        }
    }
}

// Tab completion dummy
static void le_tab_complete(char* buf, int bufsize, int *buflen, int *pos) {
    // pega primeira palavra antes de um espaço ou fim da linha
    // tenta completar
    int start= *pos;
    while (start>0 && !isspace((unsigned char)buf[start-1])) start--;
    int end = *pos;
    char partial[256];
    int plen = end - start;
    if (plen <=0 || plen>= (int)sizeof(partial)) return;
    strncpy(partial, &buf[start], plen);
    partial[plen]=0;

    // acha match
    const char* best=NULL;
    for (int i=0; dummy_cmds[i]; i++) {
        if (strncmp(dummy_cmds[i], partial, plen)==0) {
            best = dummy_cmds[i];
            break;
        }
    }
    if (best) {
        // completa
        int rest = (int)strlen(best) - plen;
        if (*buflen+rest < bufsize-1) {
            memmove(&buf[end+rest], &buf[end], *buflen-end+1);
            memcpy(&buf[end], &best[plen], rest);
            *buflen += rest;
            *pos = end+rest;
        }
    }
}

// Busca no histórico com Ctrl+R
// Apenas simplificado: se apertar Ctrl+R uma vez, mostra ultima linha do histórico
static void le_history_search(char* buf, int bufsize, int *buflen, int *pos, const char *prompt) {
    // simplificado: mostra a ultima entrada do histórico que contém a string atual
    if (*buflen>0 && history_count>0) {
        for (int i=history_count-1; i>=0; i--) {
            if (strstr(history[i], buf)) {
                strncpy(buf, history[i], bufsize-1);
                buf[bufsize-1]=0;
                *buflen=(int)strlen(buf);
                *pos=*buflen;
                le_reprint_prompt_and_buf(prompt, buf, *pos);
                return;
            }
        }
    }
}


int le_read_line(const char *prompt, char *out, int size) {
    char buf[2048];
    int buflen=0;
    int pos=0;
    buf[0]=0;

    // imprime prompt já
    write(STDOUT_FILENO, prompt, strlen(prompt));

    while (1) {
        unsigned char c;
        ssize_t n = read(STDIN_FILENO, &c, 1);
        if (n<=0) return -1;

        if (c == '\r' || c == '\n') {
            write(STDOUT_FILENO, "\n", 1);
            buf[buflen]='\0';
            strncpy(out, buf, size-1);
            out[size-1]='\0';
            return (int)strlen(out);
        } else if (c == 127 || c == 8) { // backspace
            if (pos>0) {
                memmove(&buf[pos-1], &buf[pos], buflen-pos+1);
                buflen--;
                pos--;
                le_reprint_prompt_and_buf(prompt, buf, pos);
            }
        } else if (c == 1) {
            // Ctrl+A
            le_cursor_start(buf,&pos);
            le_reprint_prompt_and_buf(prompt, buf, pos);
        } else if (c == 5) {
            // Ctrl+E
            le_cursor_end(buf,&pos);
            le_reprint_prompt_and_buf(prompt, buf, pos);
        } else if (c == 11) {
            // Ctrl+K
            le_kill_line_end(buf,&pos);
            buflen=(int)strlen(buf);
            le_reprint_prompt_and_buf(prompt, buf, pos);
        } else if (c == 21) {
            // Ctrl+U
            le_kill_line_start(buf,&pos);
            buflen=(int)strlen(buf);
            le_reprint_prompt_and_buf(prompt, buf, pos);
        } else if (c == 23) {
            // Ctrl+W
            le_kill_prev_word(buf,&pos);
            buflen=(int)strlen(buf);
            le_reprint_prompt_and_buf(prompt, buf, pos);
        } else if (c == 18) {
            // Ctrl+R search historico
            le_history_search(buf, sizeof(buf), &buflen, &pos, prompt);
        } else if (c == 27) {
            // possivel sequencia escape
            unsigned char seq[2];
            if (read(STDIN_FILENO, &seq[0], 1)<=0) continue;
            if (read(STDIN_FILENO, &seq[1], 1)<=0) continue;
            if (seq[0]=='[') {
                if (seq[1]=='A') {
                    // UP
                    le_history_up(buf,sizeof(buf),&buflen,&pos);
                    le_reprint_prompt_and_buf(prompt, buf, pos);
                } else if (seq[1]=='B') {
                    // DOWN
                    le_history_down(buf,sizeof(buf),&buflen,&pos);
                    le_reprint_prompt_and_buf(prompt, buf, pos);
                } else if (seq[1]=='C') {
                    // RIGHT
                    if (pos<buflen) {
                        pos++;
                        le_reprint_prompt_and_buf(prompt, buf, pos);
                    }
                } else if (seq[1]=='D') {
                    // LEFT
                    if (pos>0) {
                        pos--;
                        le_reprint_prompt_and_buf(prompt, buf, pos);
                    }
                }
            } else if (seq[0]=='O') {
                // Possivel ctrl + flechas?
                // Ignorar
            } else if (seq[0]=='f') {
                // Alt+F
                le_forward_word(buf,&pos);
                le_reprint_prompt_and_buf(prompt, buf, pos);
            } else if (seq[0]=='b') {
                // Alt+B
                le_backward_word(buf,&pos);
                le_reprint_prompt_and_buf(prompt, buf, pos);
            }
        } else if (c == 9) {
            // TAB completion
            le_tab_complete(buf, sizeof(buf), &buflen, &pos);
            le_reprint_prompt_and_buf(prompt, buf, pos);
        } else if (c>=32 && c<127) {
            if (buflen < (int)sizeof(buf)-1) {
                memmove(&buf[pos+1], &buf[pos], buflen-pos+1);
                buf[pos]=c;
                buflen++;
                pos++;
                le_reprint_prompt_and_buf(prompt, buf, pos);
            }
        }
    }
    return -1;
}

/*
#ifndef LINE_EDITOR_H
#define LINE_EDITOR_H

// cagado pra krl, mas funfa!
void le_init();
void le_add_history(const char* line);
int le_read_line(char *buf, int size);
void le_deinit();

#endif
*/

#ifndef LINE_EDITOR_H
#define LINE_EDITOR_H

// Editor de linha kkkk, com:
// - setas para navegação
// - histórico (UP/DOWN)
// - movimentação por palavras (Alt+F / Alt+B)
// - atalhos estilo Emacs (Ctrl+A, Ctrl+E, Ctrl+K, Ctrl+U)
// - tab-completion (fictícia, lista comandos dummy)
// - busca no histórico com Ctrl+R
// - suporte a multiline (quebra de linha longa)

void le_init();
void le_deinit();

// Adiciona uma linha ao histórico
void le_add_history(const char* line);

// Lê uma linha do usuário com edição avançada.
// 'prompt' será reimpresso conforme necessário.
// Retorna tamanho da linha lida ou -1 em caso de erro.
int le_read_line(const char *prompt, char *out, int size);

#endif



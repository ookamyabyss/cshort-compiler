#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include "lexer.h"

static FILE *fp = NULL;

void reinicia_lexer(const char *nome_arquivo) {
    if (fp != NULL) {
        fclose(fp);
    }
    fp = fopen(nome_arquivo, "r");
    if (fp == NULL) {
        perror("Erro ao abrir o arquivo");
    }
}

TOKEN proximo_token() {
    int c;
    TOKEN tk;

    // Pula espaços em branco
    do {
        c = fgetc(fp);
    } while (isspace(c));

    // Fim do arquivo
    if (c == EOF) {
        tk.cat = SN;
        tk.atributo.codigo = TK_EOF;
        return tk;
    }

    // Identificador: letra seguido de letras, dígitos ou _
    if (isalpha(c)) {
        int i = 0;
        tk.lexema[i++] = c;

        while (i < TAM_MAX_LEXEMA - 1) {
            c = fgetc(fp);
            if (isalnum(c) || c == '_') {
                tk.lexema[i++] = c;
            } else {
                ungetc(c, fp);
                break;
            }
        }
        tk.lexema[i] = '\0';
        tk.cat = ID;
        return tk;
    }

    // Número inteiro
    if (isdigit(c)) {
        int i = 0;
        tk.lexema[i++] = c;

        while (i < TAM_MAX_LEXEMA - 1) {
            c = fgetc(fp);
            if (isdigit(c)) {
                tk.lexema[i++] = c;
            } else {
                ungetc(c, fp);
                break;
            }
        }
        tk.lexema[i] = '\0';
        tk.cat = INTCON;
        return tk;
    }

    // Símbolos simples
    tk.cat = SN;
    tk.atributo.codigo = c;
    return tk;
}

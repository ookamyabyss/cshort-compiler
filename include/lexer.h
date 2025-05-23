#ifndef LEXER_H
#define LEXER_H

#define TAM_MAX_LEXEMA 100
#define TK_EOF -1

enum TOKEN_CAT {
    ID,
    INTCON,
    SN
};

typedef struct {
    enum TOKEN_CAT cat;
    union {
        int codigo;
        char lexema[TAM_MAX_LEXEMA];
    } atributo;
    char lexema[TAM_MAX_LEXEMA];
} TOKEN;

void reinicia_lexer(const char *nome_arquivo);
TOKEN proximo_token();

#endif

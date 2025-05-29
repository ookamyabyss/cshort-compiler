#include <stdio.h>
#include <stdlib.h>
#include "lexer.h"

int main(int argc, char* argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Uso: %s <arquivo-fonte>\n", argv[0]);
        return 1;
    }

    FILE* f = fopen(argv[1], "r");
    if (!f) {
        perror("Erro ao abrir o arquivo");
        return 1;
    }

    initLexer(f);

    Token tok;
    do {
        tok = getNextToken();
        printf("Lexema: %-12s Tipo: %2d (%s) Linha: %3d Coluna: %3d\n", tok.lexeme, tok.type, tokenTypeName(tok.type), tok.line, tok.column);
    } while (tok.type != TOKEN_EOF && tok.type != TOKEN_INVALID);

    fclose(f);
    destroyLexer();
    return 0;
}

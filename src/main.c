#include <stdio.h>
#include <stdlib.h>
#include "lexer.h"
#include "parser.h" 
#include "symbols.h"
#include "semantic.h"

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

    startParser(f);        // Etapa léxica + sintática + preenche tabela
    verificarSemantica();  // Etapa semântica
    imprimirTabela();      // Tabela

    fclose(f);
    return 0;
}

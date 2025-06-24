#include <stdio.h>
#include <stdlib.h>
#include "lexer.h"
#include "parser.h" 
#include "symbols.h"

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

    // Aqui antes era só o analisador léxico. Agora chamamos o parser completo:
    startParser(f);
    //imprimirTabela();     // imprime a tabela no fim

    //inserirSimbolo("x", "int", CLASSE_VAR, ESC_GLOBAL, 1);
    //inserirSimbolo("vet", "int", CLASSE_VETOR, ESC_GLOBAL, 10);
    //inserirSimbolo("soma", "int", CLASSE_FUNCAO, ESC_GLOBAL, 0);
    imprimirTabela();


    fclose(f);
    return 0;
}

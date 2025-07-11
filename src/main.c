#include <stdio.h>
#include <stdlib.h>

#include "lexer.h"
#include "parser.h" 
#include "symbols.h"
#include "semantic.h"

// Função principal: entrada do compilador
int main(int argc, char* argv[]) {
    // Verifica se o nome do arquivo-fonte foi fornecido como argumento
    if (argc != 2) {
        fprintf(stderr, "Uso: %s <arquivo-fonte>\n", argv[0]);
        return 1;
    }

    // Tenta abrir o arquivo fornecido para leitura
    FILE* f = fopen(argv[1], "r");
    if (!f) {
        perror("Erro ao abrir o arquivo");
        return 1;
    }

    // Inicia o parser: análise léxica, sintática e preenchimento da tabela de símbolos
    startParser(f);

    // Realiza a análise semântica sobre os símbolos e uso de identificadores
    verificarSemantica();

    // Imprime a tabela de símbolos resultante (para depuração)
    imprimirTabela();

    // Fecha o arquivo de entrada
    fclose(f);

    return 0;
}

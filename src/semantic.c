#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "semantic.h"
#include "symbols.h"

extern Escopo escopoAtual;

// Emite uma mensagem de erro semântico e encerra o compilador
void erroSemantico(const char* msg, const char* nome) {
    fprintf(stderr, "[ERRO SEMÂNTICO] %s: %s\n", nome, msg);
    exit(1);
}

// Verifica se uma variável (ou vetor) foi declarada
void verificarVariavelDeclarada(const char* nome) {
    Simbolo* s = buscarSimbolo(nome, escopoAtual);  // <- agora passando escopo
    if (s == NULL) {
        erroSemantico("Variável não declarada", nome);
    }
}

void verificarSemantica() {
    printf("[OK] Análise semântica concluída com sucesso.\n");
}


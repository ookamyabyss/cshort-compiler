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

// ✅ 1° Verifica se uma variável (ou vetor) foi declarada
void verificarVariavelDeclarada(const char* nome) {
    Simbolo* s = buscarSimbolo(nome, escopoAtual);  // <- agora passando escopo
    if (s == NULL) {
        erroSemantico("Variável não declarada", nome);
    }
}

// ✅ 2° Redeclaração de identificador (variável ou função)
void verificarRedeclaracao(const char* nome) {
    Simbolo* existente = buscarSimbolo(nome, escopoAtual);
    if (existente != NULL) {
        erroSemantico("Identificador já declarado no mesmo escopo", nome);
    }
}

void verificarSemantica() {
    printf("[OK] Análise semântica concluída com sucesso.\n");
}


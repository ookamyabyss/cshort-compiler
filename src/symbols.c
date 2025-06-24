#include <stdio.h>
#include <string.h>
#include "symbols.h"

// Tabela de símbolos implementada como array sequencial
static Simbolo tabela[MAX_TABELA];
static int nSimbolos = 0;

// Inicializa a tabela (zera o contador)
void inicializarTabela() {
    nSimbolos = 0;
}

// Insere um novo símbolo na tabela
int inserirSimbolo(const char* nome, const char* tipo, Classe classe, Escopo escopo, int tamanho) {
    // Verifica se já existe símbolo com mesmo nome e escopo
    for (int i = 0; i < nSimbolos; i++) {
        if (strcmp(tabela[i].nome, nome) == 0 && tabela[i].escopo == escopo) {
            fprintf(stderr, "Erro: símbolo '%s' já declarado neste escopo.\n", nome);
            return 0;  // erro de duplicação
        }
    }

    // Verifica limite
    if (nSimbolos >= MAX_TABELA) {
        fprintf(stderr, "Erro: tabela de símbolos cheia.\n");
        return 0;
    }

    // Preenche o símbolo
    strncpy(tabela[nSimbolos].nome, nome, sizeof(tabela[nSimbolos].nome));
    strncpy(tabela[nSimbolos].tipo, tipo, sizeof(tabela[nSimbolos].tipo));
    tabela[nSimbolos].classe = classe;
    tabela[nSimbolos].escopo = escopo;
    tabela[nSimbolos].tamanho = tamanho;

    nSimbolos++;
    return 1;  // sucesso
}

// Busca símbolo por nome e escopo exato (útil para inserção e resolução)
Simbolo* buscarSimbolo(const char* nome, Escopo escopo) {
    for (int i = nSimbolos - 1; i >= 0; i--) {
        if (strcmp(tabela[i].nome, nome) == 0 && tabela[i].escopo == escopo) {
            return &tabela[i];
        }
    }
    return NULL;
}

// Remove símbolos com escopo local (quando sair de uma função)
void limparEscopo(Escopo escopo) {
    while (nSimbolos > 0 && tabela[nSimbolos - 1].escopo == escopo) {
        nSimbolos--;
    }
}

// Imprime a tabela completa (para debug)
void imprimirTabela() {
    printf("======= TABELA DE SÍMBOLOS =======\n");
    for (int i = 0; i < nSimbolos; i++) {
        const char* classeStr;
        switch (tabela[i].classe) {
            case CLASSE_VAR: classeStr = "var"; break;
            case CLASSE_VETOR: classeStr = "vetor"; break;
            case CLASSE_FUNCAO: classeStr = "funcao"; break;
            case CLASSE_PARAM: classeStr = "param"; break;
            default: classeStr = "???";
        }

        const char* escopoStr = (tabela[i].escopo == ESC_GLOBAL) ? "global" : "local";

        printf("Nome: %-10s | Tipo: %-6s | Classe: %-6s | Escopo: %-6s | Tamanho: %d\n",
               tabela[i].nome,
               tabela[i].tipo,
               classeStr,
               escopoStr,
               tabela[i].tamanho);
    }
    printf("==================================\n");
}

void registrarVariavelGlobal(const char* tipo, const char* nome, int isVetor, int tamanho) {
    Classe classe = isVetor ? CLASSE_VETOR : CLASSE_VAR;
    if (!inserirSimbolo(nome, tipo, classe, ESC_GLOBAL, isVetor ? tamanho : 1)) {
        fprintf(stderr, "Erro ao registrar variável global: %s\n", nome);
    }
}
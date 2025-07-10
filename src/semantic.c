#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "semantic.h"
#include "symbols.h"

#include "lexer.h"  // ou tokens.h, onde está o struct Token

static const char* tipoAtribuido = NULL;     // Tipo da variável (esquerda)
static const char* tipoExpressao = NULL;     // Tipo do valor atribuído (direita)

extern Escopo escopoAtual;

// Emite uma mensagem de erro semântico e encerra o compilador
void erroSemantico(const char* msg, const char* nome) {
    fprintf(stderr, "[ERRO SEMÂNTICO] %s: %s\n", nome, msg);
    exit(1);
}

// ✅ 1° Verifica se uma variável (ou vetor) foi declarada
void verificarVariavelDeclarada(const char* nome) {
    Simbolo* s = buscarSimboloEmEscopos(nome); // <- agora passando escopo
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

void iniciarAtribuicao(const char* nome) {
    Simbolo* s = buscarSimboloEmEscopos(nome);
    if (s == NULL) {
        erroSemantico("Identificador não declarado antes da atribuição", nome);
    }

    if (s->classe == CLASSE_FUNCAO) {
        erroSemantico("Função usada como variável na atribuição", nome);
    }

    tipoAtribuido = s->tipo;
    tipoExpressao = NULL; 
}

void registrarTipoExpressao(const char* tipo) {
    tipoExpressao = tipo;
}

void verificarTipoExpr() {
    if (tipoAtribuido == NULL || tipoExpressao == NULL) return;

    if (strcmp(tipoAtribuido, tipoExpressao) != 0) {
        char msg[128];
        snprintf(msg, sizeof(msg), "Tipo incompatível na atribuição: esperado '%s', mas recebeu '%s'", tipoAtribuido, tipoExpressao);
        erroSemantico(msg, "");
    }
}

void registrarTipoConstante(Token token) {
    switch (token.type) {
        case TOKEN_INTCON:
            registrarTipoExpressao("int");
            break;
        case TOKEN_REALCON:
            registrarTipoExpressao("float");
            break;
        case TOKEN_CHARCON:
        case TOKEN_CHARCON_0:
        case TOKEN_CHARCON_N:
            registrarTipoExpressao("char");
            break;
        case TOKEN_BOOLCON:
            registrarTipoExpressao("bool");
            break;
        default:
            break;
    }
}

void verificarSemantica() {
    printf("[OK] Análise semântica concluída com sucesso.\n");
}

void analisarTokenAtual(Token token) {
    // Constante literal? Registra o tipo normalmente
    registrarTipoConstante(token);

    // Identificador? Pode ser variável OU função chamada numa expressão
    if (token.type == TOKEN_ID) {
        Simbolo* s = buscarSimboloEmEscopos(token.lexeme);  // <-- troca feita aqui!

        if (s == NULL) {
            erroSemantico("Identificador usado mas não declarado", token.lexeme);
        }

        // Considera que o uso é numa expressão
        registrarTipoExpressao(s->tipo);
    }
}

void registrarChamadaDeFuncao(const char* nome) {
    Simbolo* s = buscarSimboloEmEscopos(nome);
    if (s == NULL) {
        erroSemantico("Função chamada mas não declarada", nome);
    }
    if (s->classe != CLASSE_FUNCAO) {
        erroSemantico("Identificador chamado como função, mas não é uma função", nome);
    }
    registrarTipoExpressao(s->tipo); // permite verificar o tipo de retorno em atribuições
}



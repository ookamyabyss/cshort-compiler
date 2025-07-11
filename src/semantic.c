#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "semantic.h"
#include "symbols.h"
#include "lexer.h" 

// Armazena o tipo da variável no lado esquerdo da atribuição 
static const char* tipoAtribuido = NULL;

// Armazena o tipo da expressão do lado direito da atribuição 
static const char* tipoExpressao = NULL;

// Escopo atual de análise (global ou local)
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

// Inicia uma operação de atribuição, armazenando o tipo da variável alvo
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

// Registra o tipo da expressão analisada (para posterior verificação)
void registrarTipoExpressao(const char* tipo) {
    tipoExpressao = tipo;
}

// ✅ 3° Verifica tipo compatível na atribuição (esquerda vs direita)
void verificarTipoExpr() {
    if (tipoAtribuido == NULL || tipoExpressao == NULL) return;

    if (strcmp(tipoAtribuido, tipoExpressao) != 0) {
        char msg[128];
        snprintf(msg, sizeof(msg), "Tipo incompatível na atribuição: esperado '%s', mas recebeu '%s'", tipoAtribuido, tipoExpressao);
        erroSemantico(msg, "");
    }
}

// Determina o tipo de uma constante literal e registra automaticamente
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

// Finaliza a análise semântica com mensagem de sucesso (placeholder)
void verificarSemantica() {
    printf("[OK] Análise semântica concluída com sucesso.\n");
}

// Avalia o token atual e registra tipo se aplicável (constante ou id)
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

// ✅ 4° Verifica se identificador chamado é uma função válida
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
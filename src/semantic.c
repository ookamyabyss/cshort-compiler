#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "semantic.h"
#include "symbols.h"
#include "lexer.h" 
#include "parser.h"

// Armazena o tipo da variável no lado esquerdo da atribuição 
static const char* tipoAtribuido = NULL;

// Armazena o tipo da expressão do lado direito da atribuição 
static const char* tipoExpressao = NULL;

// Escopo atual de análise (global ou local)
extern Escopo escopoAtual;

// ==============================================
// INTERFACE DO ANALISADOR SEMÂNTICO - C.SHORT
// ==============================================

// ----------------------------------------------
// Mensagens de erro e finalização
// ----------------------------------------------

// Emite uma mensagem de erro semântico e encerra o compilador
void erroSemantico(const char* msg, const char* nome) {
    fprintf(stderr, "[ERRO SEMÂNTICO] %s: %s\n", nome, msg);
    exit(1);
}

// Finaliza a análise semântica com mensagem de sucesso (placeholder)
void verificarSemantica() {
    printf("[OK] Análise semântica concluída com sucesso.\n");
}

// ----------------------------------------------
// 1. Declaração e uso de variáveis
// ----------------------------------------------

// Verifica se uma variável (ou vetor) foi previamente declarada
void verificarVariavelDeclarada(const char* nome) {
    Simbolo* s = buscarSimboloEmEscopos(nome); // <- agora passando escopo
    if (s == NULL) {
        erroSemantico("Variável não declarada", nome);
    }
}

// Verifica se identificador já foi declarado no mesmo escopo
void verificarRedeclaracao(const char* nome) {
    Simbolo* existente = buscarSimbolo(nome, escopoAtual);

    if (existente != NULL) {
        // Se for função:
        if (existente->classe == CLASSE_FUNCAO) {
            // Permite se ainda não foi definida (ou seja, é um protótipo)
            if (!existente->foiDefinida) {
                return;  // ok, vai ser marcada como definida depois
            }
        }

        // Caso contrário, é erro
        erroSemantico("Identificador já declarado no mesmo escopo", nome);
    }
}

// Inicia verificação de atribuição (armazenando o tipo da variável à esquerda)
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

// Registra o tipo da expressão analisada (lado direito da atribuição)
void registrarTipoExpressao(const char* tipo) {
    tipoExpressao = tipo;
}

// Verifica se tipos na atribuição (esquerda e direita) são compatíveis
void verificarTipoExpr() {
    if (tipoAtribuido == NULL || tipoExpressao == NULL) return;

    if (strcmp(tipoAtribuido, tipoExpressao) != 0) {
        char msg[128];
        snprintf(msg, sizeof(msg), "Tipo incompatível na atribuição: esperado '%s', mas recebeu '%s'", tipoAtribuido, tipoExpressao);
        erroSemantico(msg, "");
    }
}

// Registra tipo de constante literal (int, float, char, bool)
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

// Analisa token atual e registra tipo, se aplicável (constantes ou identificadores)
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

// ----------------------------------------------
// 2. Funções - declarações e uso
// ----------------------------------------------

// Verifica se identificador chamado é uma função válida
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

// Verifica se definição de função está correta e marca como "definida"
void verificarDefinicaoDeFuncao(const char* nome) {
    Simbolo* s = buscarSimbolo(nome, ESC_GLOBAL);

    if (s != NULL) {
        if (s->classe != CLASSE_FUNCAO) {
            erroSemantico("Identificador já declarado como não função", nome);
        }
        if (s->foiDefinida) {
            erroSemantico("Função já foi definida anteriormente", nome);
        }

        // Protótipo já existia, marca como definida agora
        s->foiDefinida = true;
        return;
    }

    // Se não existia antes, é uma definição nova
    int ok = inserirSimbolo(nome, "tipo", CLASSE_FUNCAO, ESC_GLOBAL, 0);
    if (!ok) {
        erroSemantico("Erro ao definir função", nome);
    }

    // Marcar como definida (você pode acessar diretamente o último símbolo inserido)
    tabelaSimbolos[numSimbolos - 1].foiDefinida = true;
}

// Verifica se assinatura da definição bate com o protótipo anterior
void verificarAssinaturaCompatível(const char* nome, const char* tipoRetorno, int nParams, char tiposParams[][10]) {
    Simbolo* s = buscarSimbolo(nome, ESC_GLOBAL);
    if (!s || s->classe != CLASSE_FUNCAO) return;
   
    if (strcmp(s->tipo, tipoRetorno) != 0) {
        erroSemantico("Tipo de retorno da definição não bate com o protótipo", nome);
    }

    if (s->nParams != nParams) {
        erroSemantico("Número de parâmetros da definição não bate com o protótipo", nome);
    }

    for (int i = 0; i < nParams; i++) {
        if (strcmp(s->tiposParams[i], tiposParams[i]) != 0) {
            erroSemantico("Tipo de parâmetro incompatível com o protótipo", nome);
        }
    }
}

// Verifica se há parâmetro repetido na lista de parâmetros formais
void verificarParametroRepetido(const char* nome) {
    for (int i = 0; i < numParamsTemp; i++) {
        if (strcmp(nomesParamsTemp[i], nome) == 0) {
            erroSemantico("Parâmetro repetido na lista de parâmetros formais", nome);
        }
    }
}

// Verifica se função sem parâmetros declarou `void` explicitamente
void verificarVoidEmFuncaoSemParametros(int nParams, char tiposParams[][10], const char* nome) {
    if (nParams == 0) {
        erroSemantico("Função sem parâmetros deve declarar void explicitamente", nome);
    }

    if (nParams == 1 && strcmp(tiposParams[0], "void") == 0) {
        return; // ok
    }
}
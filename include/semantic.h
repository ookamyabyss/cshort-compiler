#ifndef SEMANTIC_H
#define SEMANTIC_H

#include "lexer.h"  

// ============================
// Interface do Analisador Semântico
// ============================

// Emite uma mensagem de erro semântico e encerra o compilador
void erroSemantico(const char* msg, const char* nome);

// ✅ 1. Verifica se uma variável (ou vetor) foi declarada previamente
void verificarVariavelDeclarada(const char* nome);

// ✅ 2. Verifica se o identificador já foi declarado no mesmo escopo (redeclarado)
void verificarRedeclaracao(const char* nome);

// Finaliza a análise semântica com mensagem de sucesso (placeholder)
void verificarSemantica();

// ✅ 3° Verifica tipo compatível na atribuição (esquerda vs direita)
void verificarTipoExpr();

// Registra o tipo da expressão analisada (para posterior verificação)
void registrarTipoExpressao(const char* tipo);

// Determina o tipo de uma constante literal e registra automaticamente
void registrarTipoConstante(Token token);

// Avalia o token atual e registra tipo se aplicável (constante ou id)
void analisarTokenAtual(Token token);

// Inicia uma operação de atribuição, armazenando o tipo da variável alvo
void iniciarAtribuicao(const char* nome);

// ✅ 4° Verifica se identificador chamado é uma função válida
void registrarChamadaDeFuncao(const char* nome);

void verificarDefinicaoDeFuncao(const char* nome);

void verificarAssinaturaCompatível(const char* nome, const char* tipoRetorno, int nParams, char tiposParams[][10]);

void verificarParametroRepetido(const char* nome);

void verificarVoidEmFuncaoSemParametros(int nParams, char tiposParams[][10], const char* nome);

#endif
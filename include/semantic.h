#ifndef SEMANTIC_H
#define SEMANTIC_H

#include "lexer.h"  

// ==============================================
// INTERFACE DO ANALISADOR SEMÂNTICO - C.SHORT
// ==============================================

// ----------------------------------------------
// Mensagens de erro e finalização
// ----------------------------------------------

// Emite uma mensagem de erro semântico e encerra o compilador
void erroSemantico(const char* msg, const char* nome);

// Finaliza a análise semântica com mensagem de sucesso (placeholder)
void verificarSemantica();

// ----------------------------------------------
// 1. Declaração e uso de variáveis
// ----------------------------------------------

// Verifica se uma variável (ou vetor) foi previamente declarada
void verificarVariavelDeclarada(const char* nome);

// Verifica se identificador já foi declarado no mesmo escopo
void verificarRedeclaracao(const char* nome);

// Inicia verificação de atribuição (armazenando o tipo da variável à esquerda)
void iniciarAtribuicao(const char* nome);

// Registra o tipo da expressão analisada (lado direito da atribuição)
void registrarTipoExpressao(const char* tipo);

// Verifica se tipos na atribuição (esquerda e direita) são compatíveis
void verificarTipoExpr();

// Registra tipo de constante literal (int, float, char, bool)
void registrarTipoConstante(Token token);

// Analisa token atual e registra tipo, se aplicável (constantes ou identificadores)
void analisarTokenAtual(Token token);

// ----------------------------------------------
// 2. Funções - declarações e uso
// ----------------------------------------------

// Verifica se identificador chamado é uma função válida
void registrarChamadaDeFuncao(const char* nome);

// Verifica se definição de função está correta e marca como "definida"
void verificarDefinicaoDeFuncao(const char* nome);

// Verifica se assinatura da definição bate com o protótipo anterior
void verificarAssinaturaCompatível(const char* nome, const char* tipoRetorno, int nParams, char tiposParams[][10]);

// Verifica se há parâmetro repetido na lista de parâmetros formais
void verificarParametroRepetido(const char* nome);

// Verifica se função sem parâmetros declarou `void` explicitamente
void verificarVoidEmFuncaoSemParametros(int nParams, char tiposParams[][10], const char* nome);

void garantirTipoDefinido(const char* tipo, const char* nome);

bool tiposSaoCompatíveis(const char* tipo1, const char* tipo2);

#endif
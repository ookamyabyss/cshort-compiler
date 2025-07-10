#ifndef SEMANTIC_H
#define SEMANTIC_H

#include "lexer.h"

// Emite uma mensagem de erro semântico e encerra o compilador
void erroSemantico(const char* msg, const char* nome);

// ✅ 1° Verifica se uma variável (ou vetor) foi declarada
void verificarVariavelDeclarada(const char* nome);

// ✅ 2° Redeclaração de identificador (variável ou função)
void verificarRedeclaracao(const char* nome);

void verificarSemantica();

void verificarTipoExpr();

void registrarTipoExpressao(const char* tipo); 

void registrarTipoConstante(Token token);

void analisarTokenAtual(Token token);

void iniciarAtribuicao(const char* nome);

void registrarChamadaDeFuncao(const char* nome);

#endif // SEMANTIC_H
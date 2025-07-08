#ifndef SEMANTIC_H
#define SEMANTIC_H

// Emite uma mensagem de erro semântico e encerra o compilador
void erroSemantico(const char* msg, const char* nome);

// ✅ 1° Verifica se uma variável (ou vetor) foi declarada
void verificarVariavelDeclarada(const char* nome);

// ✅ 2° Redeclaração de identificador (variável ou função)
void verificarRedeclaracao(const char* nome);

void verificarSemantica();

#endif // SEMANTIC_H

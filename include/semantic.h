#ifndef SEMANTIC_H
#define SEMANTIC_H

// Emite uma mensagem de erro semântico e encerra o compilador
void erroSemantico(const char* msg, const char* nome);

// Verifica se uma variável (ou vetor) foi declarada
void verificarVariavelDeclarada(const char* nome);

void verificarSemantica();

#endif // SEMANTIC_H

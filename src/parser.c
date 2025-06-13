#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "parser.h"
#include "lexer.h"

// Token atual (lookahead)
static Token currentToken;

// Avança para o próximo token
static void advance() {
    currentToken = getNextToken();
}

// Verifica e consome o token esperado
static void expect(TokenType expected) {
    if (currentToken.type == expected) {
        advance();
    } else {
        fprintf(stderr, "[ERRO] Esperado token '%s' na linha %d, coluna %d, mas encontrado '%s'.\n",
                tokenTypeName(expected), currentToken.line, currentToken.column, currentToken.lexeme);
        exit(EXIT_FAILURE);
    }
}

// Ponto de entrada do parser
void startParser(FILE* f) {
    initLexer(f);
    advance(); // inicializa lookahead
    parseProg();
    printf("[OK] Análise sintática concluída com sucesso.\n");
    destroyLexer();
}

// prog ::= { decl ';' | func }
void parseProg() {
    while (currentToken.type != TOKEN_EOF) {
        if (currentToken.type == TOKEN_KEYWORD_INT || currentToken.type == TOKEN_KEYWORD_CHAR || currentToken.type == TOKEN_KEYWORD_FLOAT) {
            advance(); // consome tipo (ex: int)
            if (currentToken.type == TOKEN_ID) {
                advance(); // consome identificador
                if (currentToken.type == TOKEN_LPAREN) {
                    // funcao
                    printf("[INFO] Reconhecida função.\n");
                    // parseFunc(); ← vai implementar depois
                    // aqui por enquanto só consome os próximos tokens até {
                    while (currentToken.type != TOKEN_LBRACE && currentToken.type != TOKEN_EOF)
                        advance();
                } else {
                    // declaracao
                    printf("[INFO] Reconhecida declaração.\n");
                    // parseDecl(); ← vai implementar depois
                    while (currentToken.type != TOKEN_SEMICOLON && currentToken.type != TOKEN_EOF)
                        advance();
                    expect(TOKEN_SEMICOLON);
                }
            } else {
                fprintf(stderr, "[ERRO] Esperado identificador após tipo na linha %d, coluna %d.\n", currentToken.line, currentToken.column);
                exit(EXIT_FAILURE);
            }
        } else {
            fprintf(stderr, "[ERRO] Comando inesperado na linha %d, coluna %d: %s\n",
                    currentToken.line, currentToken.column, currentToken.lexeme);
            exit(EXIT_FAILURE);
        }
    }
}


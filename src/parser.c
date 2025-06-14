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

static void erro(const char* msg) {
    fprintf(stderr, "[ERRO] %s na linha %d, coluna %d. Token: '%s'\n",
            msg, currentToken.line, currentToken.column, currentToken.lexeme);
    exit(EXIT_FAILURE);
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
        if (isTipo(currentToken.type)) {
            // Pode ser declaração ou função
            // A decisão fica para parseDecl()
            parseDecl();
            
        } else if (currentToken.type == TOKEN_KEYWORD_VOID) {
            // Função void
            parseDecl();  // sua parseDecl já trata função void
            // em parseDecl, para função, não espera ';'
            // Você pode ajustar parseDecl pra consumir ';' só se for variável
        } else {
            erro("Esperado tipo ou void");
        }
    }
}

void parseDecl() {
    if (isTipo(currentToken.type)) {
        parseTipo();

        if (currentToken.type == TOKEN_ID) {
            advance();

            if (currentToken.type == TOKEN_LPAREN) {
                // Função
                advance();
                parseTiposParam();
                expect(TOKEN_RPAREN);

                while (currentToken.type == TOKEN_COMMA) {
                    advance();
                    expect(TOKEN_ID);
                    expect(TOKEN_LPAREN);
                    parseTiposParam();
                    expect(TOKEN_RPAREN);
                }

                if (currentToken.type == TOKEN_SEMICOLON) {
                    advance();
                    printf("[INFO] Reconhecida função com tipo.\n");
                } else if (currentToken.type == TOKEN_LBRACE) {
                    parseFunc();
                } else {
                    erro("Esperado ';' ou '{' após declaração de função");
                }
            } else {
                // Variável
                parseDeclVarResto();
                expect(TOKEN_SEMICOLON);
                printf("[INFO] Reconhecida declaração.\n");
            }
        } else {
            erro("Esperado identificador após tipo");
        }
    }
    else if (currentToken.type == TOKEN_KEYWORD_VOID) {
        // semelhante para void
        advance();
        expect(TOKEN_ID);
        expect(TOKEN_LPAREN);
        parseTiposParam();
        expect(TOKEN_RPAREN);

        while (currentToken.type == TOKEN_COMMA) {
            advance();
            expect(TOKEN_ID);
            expect(TOKEN_LPAREN);
            parseTiposParam();
            expect(TOKEN_RPAREN);
        }

        if (currentToken.type == TOKEN_SEMICOLON) {
            advance();
            printf("[INFO] Reconhecida função void.\n");
        } else if (currentToken.type == TOKEN_LBRACE) {
            parseFunc();
        } else {
            erro("Esperado ';' ou '{' após declaração de função void");
        }
    }
    else {
        erro("Esperado tipo ou void na declaração");
    }
}

void parseTipo() {
    if (isTipo(currentToken.type)) {
        advance();
    } else {
        erro("Esperado tipo (int, float, char, bool)");
    }
}

void parseTiposParam() {
    if (currentToken.type == TOKEN_KEYWORD_VOID) {
        advance();
        return;
    }

    parseTipo();

    parseParam();

    while (currentToken.type == TOKEN_COMMA) {
        advance();
        parseTipo();
        parseParam();
    }
}

void parseFunc() {
    // Pula o corpo da função por enquanto
    if (currentToken.type == TOKEN_LBRACE) {
        int abre = 1;
        advance();
        while (abre > 0 && currentToken.type != TOKEN_EOF) {
            if (currentToken.type == TOKEN_LBRACE) abre++;
            else if (currentToken.type == TOKEN_RBRACE) abre--;
            advance();
        }
        if (abre != 0)
            erro("Bloco da função não fechado");
    } else {
        erro("Esperado '{' no início da função");
    }
}

int isTipo(TokenType t) {
    return t == TOKEN_KEYWORD_INT ||
           t == TOKEN_KEYWORD_CHAR ||
           t == TOKEN_KEYWORD_FLOAT ||
           t == TOKEN_KEYWORD_BOOL;
}

void parseDeclVarResto() {
    if (currentToken.type == TOKEN_LBRACK) {
        advance();
        if (currentToken.type == TOKEN_INTCON) {
            advance();
            expect(TOKEN_RBRACK);
        } else {
            erro("Esperado número inteiro dentro dos colchetes");
        }
    }

    while (currentToken.type == TOKEN_COMMA) {
        advance();
        expect(TOKEN_ID);

        if (currentToken.type == TOKEN_LBRACK) {
            advance();
            if (currentToken.type == TOKEN_INTCON) {
                advance();
                expect(TOKEN_RBRACK);
            } else {
                erro("Esperado número inteiro dentro dos colchetes após vírgula");
            }
        }
    }
}

void parseParam() {
    if (currentToken.type == TOKEN_AMPERSAND) {
        advance();
        expect(TOKEN_ID);
    } else if (currentToken.type == TOKEN_ID) {
        advance();
        if (currentToken.type == TOKEN_LBRACK) {
            advance();
            expect(TOKEN_RBRACK);
        }
    } else {
        erro("Esperado identificador, &identificador ou identificador[] no parâmetro");
    }
}
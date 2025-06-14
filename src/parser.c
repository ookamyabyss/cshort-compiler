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
            parseDecl();
        } else if (currentToken.type == TOKEN_KEYWORD_VOID) {
            // Função void
            parseDecl();
        } else {
            erro("Esperado tipo ou void");
        }
    }
}

// decl ::= tipo decl_var { ',' decl_var} 
//      | tipo id '(' tipos_param')' { ',' id '(' tipos_param')' } 
//      | void id '(' tipos_param')' { ',' id '(' tipos_param')' }
void parseDecl() {
    if (isTipo(currentToken.type)) {
        parseTipo();

        if (currentToken.type == TOKEN_ID) {
            char nomeFunc[256];
            strncpy(nomeFunc, currentToken.lexeme, sizeof(nomeFunc));
            nomeFunc[sizeof(nomeFunc) - 1] = '\0';

            advance();

            if (currentToken.type == TOKEN_LPAREN) {
                // função com tipo
                printf("[DECL_FUNCAO] Função com tipo reconhecida: %s\n", nomeFunc);
                advance();
                parseTiposParam();
                expect(TOKEN_RPAREN);

                while (currentToken.type == TOKEN_COMMA) {
                    advance();

                    if (currentToken.type != TOKEN_ID) {
                        erro("Esperado identificador após ',' na declaração de função");
                    }

                    char funcName[256];
                    strncpy(funcName, currentToken.lexeme, sizeof(funcName));
                    funcName[sizeof(funcName) - 1] = '\0';

                    advance(); // consome o ID
                    expect(TOKEN_LPAREN);
                    parseTiposParam();
                    expect(TOKEN_RPAREN);

                    printf("[DECL_FUNCAO] Função adicional reconhecida: %s\n", funcName);
                }

                if (currentToken.type == TOKEN_SEMICOLON) {
                    advance();
                } else if (currentToken.type == TOKEN_LBRACE) {
                    parseFunc();
                } else {
                    erro("Esperado ';' ou '{' após declaração de função");
                }
            } else {
                // declaração variável
                printf("[DECL] Reconhecida declaração de variável (primeiro ID: %s)\n", nomeFunc);
                parseDeclVarPrimeiro();
                parseDeclVarResto();
                expect(TOKEN_SEMICOLON);
            }
        } else {
            erro("Esperado identificador após tipo");
        }

    } else if (currentToken.type == TOKEN_KEYWORD_VOID) {
        expect(TOKEN_KEYWORD_VOID);

        char nomeFunc[256];
        if (currentToken.type == TOKEN_ID) {
            strncpy(nomeFunc, currentToken.lexeme, sizeof(nomeFunc));
            nomeFunc[sizeof(nomeFunc) - 1] = '\0';
        }

        expect(TOKEN_ID);
        printf("[DECL_FUNCAO_VOID] Função void reconhecida: %s\n", nomeFunc);
        expect(TOKEN_LPAREN);
        parseTiposParam();
        expect(TOKEN_RPAREN);

        while (currentToken.type == TOKEN_COMMA) {
            advance();
            expect(TOKEN_ID);
            printf("[DECL_FUNCAO_VOID] Função void adicional: %s\n", currentToken.lexeme);
            expect(TOKEN_LPAREN);
            parseTiposParam();
            expect(TOKEN_RPAREN);
        }

        if (currentToken.type == TOKEN_SEMICOLON) {
            advance();
        } else if (currentToken.type == TOKEN_LBRACE) {
            parseFunc();
        } else {
            erro("Esperado ';' ou '{' após declaração de função void");
        }

    } else {
        erro("Esperado tipo ou void na declaração");
    }
}

// decl_var ::= id [ '[' intcon ']' ]
void parseDeclVar() {
    char id_lexeme[256];
    strncpy(id_lexeme, currentToken.lexeme, sizeof(id_lexeme));
    id_lexeme[sizeof(id_lexeme) - 1] = '\0';

    expect(TOKEN_ID);
    printf("[DECL_VAR] Reconhecida variável: %s\n", id_lexeme);

    if (currentToken.type == TOKEN_LBRACK) {
        advance();
        if (currentToken.type == TOKEN_INTCON) {
            printf("[DECL_VAR] Vetor de tamanho: %s\n", currentToken.lexeme);
            advance();
            expect(TOKEN_RBRACK);
        } else {
            erro("Esperado número inteiro dentro dos colchetes");
        }
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
    // Lista vazia de parâmetros (ex: função com parênteses vazios)
    if (currentToken.type == TOKEN_RPAREN) {
        return; // lista vazia
    }

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

void parseDeclVarPrimeiro() {
    // Aqui o id já foi consumido no parseDecl
    printf("[DECL_VAR] Reconhecida variável (primeira)\n");
    if (currentToken.type == TOKEN_LBRACK) {
        advance();
        printf("[DECL_VAR] Vetor com tamanho: %s\n", currentToken.lexeme);
        expect(TOKEN_INTCON);
        expect(TOKEN_RBRACK);
    }
}

void parseDeclVarResto() {
    while (currentToken.type == TOKEN_COMMA) {
        advance();
        expect(TOKEN_ID);
        printf("[DECL_VAR] Reconhecida variável (extra)\n");
        parseDeclVarPrimeiro();
    }
}

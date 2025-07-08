#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include "parser.h"
#include "lexer.h"
#include "symbols.h"
#include "semantic.h"

// ==============================
// Variáveis globais
// ==============================

static Token currentToken;     // Lookahead atual
static Token backupToken;      // Token salvo para "voltar"
static Token pushedToken;      // Token empurrado manualmente
static int hasPushedToken = 0;
static bool tokenBack = false;

void obterTipoString(char* dest);

// ==============================
// Controle de Tokens
// ==============================

// Avança para o próximo token.
void advance() {
    if (tokenBack) {
        tokenBack = false;
        // Usa o backupToken em vez de pegar novo token
        currentToken = backupToken;
    } else {
        currentToken = getNextToken();
    }
}

// Salva um token para ser lido novamente no próximo `advance()`.
void pushBackToken(Token t) {
    pushedToken = t;
    hasPushedToken = 1;
}

// Marca que o token atual deve ser reutilizado.
void ungetToken(Token t) {
    backupToken = t;
    tokenBack = true;
}

// ==============================
// Erros
// ==============================

// Função para reportar erros sintáticos e encerrar.
static void parseError(const char* message) {
    fprintf(stderr, "[ERRO SINTÁTICO] %s. Encontrado '%s' (tipo %d) na linha %d, coluna %d.\n",
            message, currentToken.lexeme, currentToken.type, currentToken.line, currentToken.column);
    exit(EXIT_FAILURE);
}

// Espera e consome um token do tipo esperado.
void parseEat(int expectedType) {
    if (currentToken.type == expectedType) {
        advance();
    } else {
        fprintf(stderr, "[ERRO SINTÁTICO] Esperado token do tipo %d, mas encontrado '%s' (linha %d, coluna %d)\n",
                expectedType, currentToken.lexeme, currentToken.line, currentToken.column);
        exit(EXIT_FAILURE); // encerra o programa.
    }
}

// ==============================
// Entrada do Parser
// ==============================

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
            parseError("Esperado tipo ou void");
        }
    }
}

// decl ::= tipo decl_var {...} | tipo id(...) {...} | void id(...) {...}
void parseDecl() {
    if (isTipo(currentToken.type)) {
        char tipoStr[10];
        obterTipoString(tipoStr);  // ← Essa função pega o tipo em string
        parseTipo();

        if (currentToken.type == TOKEN_ID) {
            char nomeFunc[256];
            strncpy(nomeFunc, currentToken.lexeme, sizeof(nomeFunc));
            nomeFunc[sizeof(nomeFunc) - 1] = '\0';
            //Token idToken = currentToken;

            advance();

            if (currentToken.type == TOKEN_LPAREN) {

                // ✅ Verificação semântica
                verificarRedeclaracao(nomeFunc);

                // função com tipo
                registrarFuncao(tipoStr, nomeFunc);
                printf("[DECL_FUNCAO] Função com tipo reconhecida: %s\n", nomeFunc);
                advance();
                parseTiposParam();
                parseEat(TOKEN_RPAREN);

                while (currentToken.type == TOKEN_COMMA) {
                    advance();
                    parseEat(TOKEN_ID);
                    printf("[DECL_FUNCAO] Função adicional reconhecida: %s\n", currentToken.lexeme);
                    parseEat(TOKEN_LPAREN);
                    parseTiposParam();
                    parseEat(TOKEN_RPAREN);
                }

            if (currentToken.type == TOKEN_SEMICOLON) {
                advance();
            } else if (currentToken.type == TOKEN_LBRACE) {
                parseFunc();
            } else {
                parseError("Esperado ';' ou '{' após declaração de função");
            }
            } else {
                // declaração variável
                printf("[DECL] Reconhecida declaração de variável (primeiro ID: %s)\n", nomeFunc);

                int isVetor = 0;
                int tamanho = 1;

                if (currentToken.type == TOKEN_LBRACK) {
                    advance();
                    if (currentToken.type == TOKEN_INTCON) {
                        tamanho = atoi(currentToken.lexeme);
                        isVetor = 1;
                        printf("[DECL_VAR] Vetor de tamanho: %s\n", currentToken.lexeme);
                        advance();
                        parseEat(TOKEN_RBRACK);
                    } else {
                        parseError("Esperado número inteiro dentro dos colchetes após o identificador");
                    }
                }

                // ✅ Verificação semântica
                verificarRedeclaracao(nomeFunc);

                registrarVariavelGlobal(tipoStr, nomeFunc, isVetor, tamanho);


                // Verifica se há vetor após o primeiro identificador
                if (currentToken.type == TOKEN_LBRACK) {
                    advance(); // consome '['

                    if (currentToken.type == TOKEN_INTCON) {
                        printf("[DECL_VAR] Vetor de tamanho: %s\n", currentToken.lexeme);
                        advance(); // consome número
                        parseEat(TOKEN_RBRACK); // consome ']'
                    } else {
                        parseError("Esperado número inteiro dentro dos colchetes após o identificador");
                    }
                }

                // Agora trata as outras variáveis separadas por vírgula
                while (currentToken.type == TOKEN_COMMA) {
                    advance(); // consome ','
                    parseDeclVar(tipoStr, ESC_GLOBAL); // consome próximo id e vetor se tiver
                }

                parseEat(TOKEN_SEMICOLON);

            }
        } else {
            parseError("Esperado identificador após tipo");
        }

    } else if (currentToken.type == TOKEN_KEYWORD_VOID) {
        parseEat(TOKEN_KEYWORD_VOID);

        char nomeFunc[256];
        if (currentToken.type == TOKEN_ID) {
            strncpy(nomeFunc, currentToken.lexeme, sizeof(nomeFunc));
            nomeFunc[sizeof(nomeFunc) - 1] = '\0';
        }

        parseEat(TOKEN_ID);

        printf("[DECL_FUNCAO_VOID] Função void reconhecida: %s\n", nomeFunc);
        
        // ✅ Verificação semântica
        verificarRedeclaracao(nomeFunc);

        registrarFuncao("void", nomeFunc);
        parseEat(TOKEN_LPAREN);
        parseTiposParam();
        parseEat(TOKEN_RPAREN);

        while (currentToken.type == TOKEN_COMMA) {
            advance();
            parseEat(TOKEN_ID);
            printf("[DECL_FUNCAO_VOID] Função void adicional: %s\n", currentToken.lexeme);
            parseEat(TOKEN_LPAREN);
            parseTiposParam();
            parseEat(TOKEN_RPAREN);
        }

        if (currentToken.type == TOKEN_SEMICOLON) {
            advance();
        } else if (currentToken.type == TOKEN_LBRACE) {
            parseFunc();
        } else {
            parseError("Esperado ';' ou '{' após declaração de função void");
        }

    } else {
        parseError("Esperado tipo ou void na declaração");
    }
}

// decl_var ::= id [ '[' intcon ']' ]
void parseDeclVar(const char* tipo, Escopo escopo) {
    char nomeVar[256];
    int isVetor = 0;
    int tamanho = 1;

    strncpy(nomeVar, currentToken.lexeme, sizeof(nomeVar));
    nomeVar[sizeof(nomeVar) - 1] = '\0';

    parseEat(TOKEN_ID);
    printf("[DECL_VAR] Reconhecida variável: %s\n", nomeVar);

    if (currentToken.type == TOKEN_LBRACK) {
        isVetor = 1;
        advance();
        if (currentToken.type == TOKEN_INTCON) {
            tamanho = atoi(currentToken.lexeme);
            printf("[DECL_VAR] Vetor de tamanho: %d\n", tamanho);
            advance();
            parseEat(TOKEN_RBRACK);
        } else {
            parseError("Esperado número inteiro dentro dos colchetes");
        }
    }

    // ✅ Verificação semântica
    verificarRedeclaracao(nomeVar);

    registrarVariavelGlobal(tipo, nomeVar, isVetor, tamanho);
}

// tipo ::= char | int | float | bool 
void parseTipo() {
    if (isTipo(currentToken.type)) {
        advance();
    } else {
        parseError("Esperado tipo (int, float, char, bool)");
    }
}

// tipos_param ::= void | tipo (id | &id | id[]){, tipo (...)}
void parseTiposParam() {
    if (currentToken.type == TOKEN_RPAREN) {
        return;
    }

    if (currentToken.type == TOKEN_KEYWORD_VOID) {
        advance();

        if (currentToken.type != TOKEN_RPAREN) {
            parseError("Token 'void' não pode ser seguido por outros parâmetros");
        }

        return;
    }

    parseTipoParam();  // consome tipo e param juntos

    while (currentToken.type == TOKEN_COMMA) {
        advance();
        parseTipoParam();  
    }
}

// func ::= tipo/void id(...) '{' {decl_var} {cmd} '}' 
void parseFunc() {

    parseEat(TOKEN_LBRACE);

    while (isTipo(currentToken.type)) {
        char tipoStr[10];
        obterTipoString(tipoStr);  // ← Isso obtém o tipo em string
        parseTipo();
        parseDeclVarPrimeiro(tipoStr, ESC_LOCAL);
        parseDeclVarResto(tipoStr, ESC_LOCAL);
        parseEat(TOKEN_SEMICOLON);
    }

    while (currentToken.type != TOKEN_RBRACE && currentToken.type != TOKEN_EOF) {
        parseCmd();
    }

    parseEat(TOKEN_RBRACE);
    limparEscopo(ESC_LOCAL);
}

// cmd ::= if, while, for, return, atrib, chamada, bloco, ';'
void parseCmd() {
    if (currentToken.type == TOKEN_KEYWORD_IF) {
        printf("[CMD] Reconhecido comando 'if'\n");
        advance();

        parseEat(TOKEN_LPAREN);
        parseExpr();
        parseEat(TOKEN_RPAREN);

        parseCmd();

        if (currentToken.type == TOKEN_KEYWORD_ELSE) {
            printf("[CMD] Reconhecido bloco 'else'\n");
            advance();
            parseCmd();
        }

    } else if (currentToken.type == TOKEN_KEYWORD_WHILE) {
        printf("[CMD] Reconhecido comando 'while'\n");
        advance();

        parseEat(TOKEN_LPAREN);
        parseExpr();
        parseEat(TOKEN_RPAREN);

        parseCmd();

    } else if (currentToken.type == TOKEN_KEYWORD_FOR) {
        printf("[CMD] Reconhecido comando 'for'\n");
        advance();

        parseEat(TOKEN_LPAREN);

        if (currentToken.type == TOKEN_ID) {
            parseAtrib();
        }
        parseEat(TOKEN_SEMICOLON);

        if (currentToken.type != TOKEN_SEMICOLON) {
            parseExpr();
        }
        parseEat(TOKEN_SEMICOLON);

        if (currentToken.type == TOKEN_ID) {
            parseAtrib();
        }
        parseEat(TOKEN_RPAREN);

        parseCmd();

    } else if (currentToken.type == TOKEN_KEYWORD_RETURN) {
        printf("[CMD] Reconhecido comando 'return'\n");
        advance();

        if (currentToken.type != TOKEN_SEMICOLON) {
            parseExpr();
        }

        parseEat(TOKEN_SEMICOLON);

    } else if (currentToken.type == TOKEN_LBRACE) {
        printf("[CMD] Bloco composto reconhecido\n");
        advance();

        while (currentToken.type != TOKEN_RBRACE && currentToken.type != TOKEN_EOF) {
            parseCmd();
        }
        parseEat(TOKEN_RBRACE);

    } else if (currentToken.type == TOKEN_SEMICOLON) {
        printf("[CMD] Comando vazio reconhecido\n");
        advance();

    } else if (currentToken.type == TOKEN_ID) {
        Token lookahead = getNextToken();
        ungetToken(lookahead);

        if (lookahead.type == TOKEN_ASSIGN || lookahead.type == TOKEN_LBRACK) {
            parseAtrib();
            parseEat(TOKEN_SEMICOLON);
            return;

        } else if (lookahead.type == TOKEN_LPAREN) {
            // chamada de função como comando
            printf("[CMD] Chamada de função reconhecida: %s\n", currentToken.lexeme);
            advance(); // consome id
            parseEat(TOKEN_LPAREN);

            if (currentToken.type != TOKEN_RPAREN) {
                parseExpr();

                while (currentToken.type == TOKEN_COMMA) {
                    advance();
                    parseExpr();
                }
            }

            parseEat(TOKEN_RPAREN);
            parseEat(TOKEN_SEMICOLON);
            return;
        } else {
            parseError("Identificador inesperado — esperada atribuição ou chamada de função");
        }

    } else {
        printf("[CMD] Comando inválido ou não tratado: token '%s'\n", currentToken.lexeme);
        parseError("Comando não reconhecido");
    }
}

// atrib ::= id [ '[' expr ']' ] = expr
void parseAtrib() {
    if (currentToken.type != TOKEN_ID) {
        parseError("Esperado identificador no início da atribuição");
        return;
    }

    // ✅ Verificação semântica
    verificarVariavelDeclarada(currentToken.lexeme);

    printf("[ATRIB] Início de atribuição: %s\n", currentToken.lexeme);
    advance();  // consome o id

    // Verifica se é uma atribuição em vetor
    if (currentToken.type == TOKEN_LBRACK) {
        printf("[ATRIB] Índice de vetor detectado\n");
        advance();  // consome '['
        parseExpr();
        parseEat(TOKEN_RBRACK);  // consome ']'
    }

    parseEat(TOKEN_ASSIGN);  // consome '='
    parseExpr();          // processa o lado direito da atribuição

    printf("[ATRIB] Atribuição completa reconhecida\n");
}

// expr ::= expr_simp [ op_rel  expr_simp ] 
void parseExpr() {
    parseExprSimp();
    if (currentToken.type == TOKEN_EQ || currentToken.type == TOKEN_NEQ ||
        currentToken.type == TOKEN_LT || currentToken.type == TOKEN_GT ||
        currentToken.type == TOKEN_LEQ || currentToken.type == TOKEN_GEQ) {
        advance(); // consome operador relacional
        parseExprSimp();
    }
    printf("[EXPR] Expressão reconhecida (expr)\n");
}

// expr_simp ::= [+ | – ] termo {(+ | – | ||) termo} 
void parseExprSimp() {
    if (currentToken.type == TOKEN_PLUS || currentToken.type == TOKEN_MINUS) {
        advance(); // consome operador unário
    }

    parseTermo();

    while (currentToken.type == TOKEN_PLUS || 
           currentToken.type == TOKEN_MINUS || 
           currentToken.type == TOKEN_OR) {
        advance(); // consome operador
        parseTermo();
    }

    printf("[EXPR] Expressão reconhecida (expr_simp)\n");
}

// termo ::= fator {(* | / | &&)  fator} 
void parseTermo() {
    parseFator();

    while (currentToken.type == TOKEN_MUL || 
           currentToken.type == TOKEN_DIV || 
           currentToken.type == TOKEN_AND) {
        advance(); // consome operador
        parseFator();
    }

    printf("[EXPR] Expressão reconhecida (termo)\n");
}

// fator ::= id[...] | constantes | chamada | (!fator)
void parseFator() {
    if (currentToken.type == TOKEN_ID) {
        Token idToken = currentToken;

        // ✅ Verificação semântica
        verificarVariavelDeclarada(idToken.lexeme);

        advance();

        if (currentToken.type == TOKEN_LBRACK) {
            advance();
            parseExpr();
            parseEat(TOKEN_RBRACK);
        }
        else if (currentToken.type == TOKEN_LPAREN) {
            advance();
            if (currentToken.type != TOKEN_RPAREN) {
                parseExpr();
                while (currentToken.type == TOKEN_COMMA) {
                    advance();
                    parseExpr();
                }
            }
            parseEat(TOKEN_RPAREN);
        }

        printf("[EXPR] Fator reconhecido: %s\n", idToken.lexeme);
    }
    else if (currentToken.type == TOKEN_INTCON || 
             currentToken.type == TOKEN_REALCON ||
             currentToken.type == TOKEN_CHARCON || 
             currentToken.type == TOKEN_CHARCON_N ||
             currentToken.type == TOKEN_CHARCON_0) {
        printf("[EXPR] Constante reconhecida: %s\n", currentToken.lexeme);
        advance();
    }
    else if (currentToken.type == TOKEN_LPAREN) {
        advance();
        parseExpr();
        parseEat(TOKEN_RPAREN);
    }
    else if (strcmp(currentToken.lexeme, "!") == 0) {
        advance();
        parseFator();
    }
    else {
        parseError("Fator inválido");
    }
}

// ==============================
// Funções auxiliares de análise
// ==============================

// Primeira variável da lista
void parseDeclVarPrimeiro(const char* tipo, Escopo escopo) {
    if (currentToken.type != TOKEN_ID) {
        parseError("Esperado identificador na declaração de variável");
    }

    char nome[256];
    int isVetor = 0;
    int tamanho = 1;

    strncpy(nome, currentToken.lexeme, sizeof(nome));
    nome[sizeof(nome) - 1] = '\0';
    advance(); // consome o ID

    printf("[DECL_VAR] Reconhecida variável: %s\n", nome);

    if (currentToken.type == TOKEN_LBRACK) {
        advance();
        if (currentToken.type == TOKEN_INTCON) {
            isVetor = 1;
            tamanho = atoi(currentToken.lexeme);
            printf("[DECL_VAR] Vetor com tamanho: %s\n", currentToken.lexeme);
            advance();
            parseEat(TOKEN_RBRACK);
        } else {
            parseError("Esperado número inteiro dentro dos colchetes");
        }
    }

    if (escopo == ESC_GLOBAL)
        registrarVariavelGlobal(tipo, nome, isVetor, tamanho);
    else
        registrarVariavelLocal(tipo, nome, isVetor, tamanho);
}

// Demais variáveis após vírgula
void parseDeclVarResto(const char* tipo, Escopo escopo) {
    while (currentToken.type == TOKEN_COMMA) {
        advance(); // consome ','

        if (currentToken.type != TOKEN_ID) {
            parseError("Esperado identificador após ','");
        }

        char nome[256];
        int isVetor = 0;
        int tamanho = 1;

        strncpy(nome, currentToken.lexeme, sizeof(nome));
        nome[sizeof(nome) - 1] = '\0';
        advance(); // consome o ID

        printf("[DECL_VAR] Reconhecida variável extra: %s\n", nome);

        if (currentToken.type == TOKEN_LBRACK) {
            advance();
            if (currentToken.type == TOKEN_INTCON) {
                isVetor = 1;
                tamanho = atoi(currentToken.lexeme);
                printf("[DECL_VAR] Vetor de tamanho: %s\n", currentToken.lexeme);
                advance();
                parseEat(TOKEN_RBRACK);
            } else {
                parseError("Esperado número inteiro dentro dos colchetes");
            }
        }

        if (escopo == ESC_GLOBAL)
            registrarVariavelGlobal(tipo, nome, isVetor, tamanho);
        else
            registrarVariavelLocal(tipo, nome, isVetor, tamanho);

    }
}

// Tipo (id | &id | id[])
void parseTipoParam() {
    if (!isTipo(currentToken.type)) {
        parseError("Esperado tipo (int, char, float, bool) no parâmetro");
    }

    char tipoStr[10];                  // ← Captura o tipo ANTES de consumir
    obterTipoString(tipoStr);
    advance(); // consome o tipo

    // Verifica se é '&' (um único token do tipo TOKEN_AND)
    int porReferencia = 0;
    if (currentToken.type == TOKEN_BITAND) {
        porReferencia = 1;
        advance(); // consome '&'
    }

    // Agora deve vir um identificador
    if (currentToken.type != TOKEN_ID) {
        parseError("Esperado identificador no parâmetro");
    }

    char nome[256];
    strncpy(nome, currentToken.lexeme, sizeof(nome));
    nome[sizeof(nome) - 1] = '\0';
    advance(); // consome ID

    // Verifica se é vetor
    int isVetor = 0;
    if (currentToken.type == TOKEN_LBRACK) {
        advance();
        parseEat(TOKEN_RBRACK);
        isVetor = 1;
    }

    // Imprime o tipo de parâmetro detectado
    if (porReferencia) {
        
        printf("[PARAM] Parâmetro por referência: &%s\n", nome);
        registrarParametro(tipoStr, nome, CLASSE_PARAM);
    } else if (isVetor) {
        
        printf("[PARAM] Parâmetro vetor: %s[]\n", nome);
        registrarParametro(tipoStr, nome, CLASSE_VETOR);
    } else {
        
        printf("[PARAM] Parâmetro comum: %s\n", nome);
        registrarParametro(tipoStr, nome, CLASSE_PARAM);
    }
}

// Lista de variáveis tipo v1, v2, v3;
void parseDeclVarLista(const char* tipo, Escopo escopo) {
    parseDeclVar(tipo, ESC_GLOBAL); // primeiro já consumido id

    while (currentToken.type == TOKEN_COMMA) {
        advance(); // consome ','
        parseDeclVar(tipo, ESC_GLOBAL); // próximo id
    }
}

// ==============================
// Utilitários de parsing
// ==============================

// verifica se t é tipo válido
int isTipo(TokenType t) {
    return t == TOKEN_KEYWORD_INT ||
           t == TOKEN_KEYWORD_CHAR ||
           t == TOKEN_KEYWORD_FLOAT ||
           t == TOKEN_KEYWORD_BOOL;
}

// verifica se t inicia comando
int isComandoInicio(TokenType t) {
    return t == TOKEN_KEYWORD_IF || t == TOKEN_KEYWORD_WHILE ||
           t == TOKEN_KEYWORD_RETURN || t == TOKEN_LBRACE ||
           t == TOKEN_ID || t == TOKEN_SEMICOLON;
}

// ==============================
// SIMBOLOS FUNC AUXILIARES
// ==============================
void obterTipoString(char* dest) {
    switch (currentToken.type) {
        case TOKEN_KEYWORD_INT:   strcpy(dest, "int"); break;
        case TOKEN_KEYWORD_FLOAT: strcpy(dest, "float"); break;
        case TOKEN_KEYWORD_CHAR:  strcpy(dest, "char"); break;
        case TOKEN_KEYWORD_BOOL:  strcpy(dest, "bool"); break;
        default: strcpy(dest, "???"); break;
    }
}

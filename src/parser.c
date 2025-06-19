#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "parser.h"
#include "lexer.h"
#include <stdbool.h>

// Token atual (lookahead)
static Token currentToken;

// Buffer para um token devolvido
Token pushedToken;
int hasPushedToken = 0;

bool tokenBack = false;
Token backupToken;

// Avança para o próximo token
void advance() {
    if (tokenBack) {
        tokenBack = false;
        // Usa o backupToken em vez de pegar novo token
        currentToken = backupToken;
    } else {
        currentToken = getNextToken();
    }
}

void pushBackToken(Token t) {
    pushedToken = t;
    hasPushedToken = 1;
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
                    expect(TOKEN_ID);
                    printf("[DECL_FUNCAO] Função adicional reconhecida: %s\n", currentToken.lexeme);
                    expect(TOKEN_LPAREN);
                    parseTiposParam();
                    expect(TOKEN_RPAREN);
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
                while (currentToken.type == TOKEN_COMMA) {
                    advance(); // consome ','
                    parseDeclVar(); // consome próximo id e opcional vetor
                }

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

// tipo ::= char | int | float | bool 
void parseTipo() {
    if (isTipo(currentToken.type)) {
        advance();
    } else {
        erro("Esperado tipo (int, float, char, bool)");
    }
}

// tipos_param ::= void 
//              | tipo (id | &&id | id '[' ']') { ','  tipo (id | &&id | id '[' ']') }
void parseTiposParam() {
    if (currentToken.type == TOKEN_RPAREN) {
        return;
    }

    if (currentToken.type == TOKEN_KEYWORD_VOID) {
        advance();

        if (currentToken.type != TOKEN_RPAREN) {
            erro("Token 'void' não pode ser seguido por outros parâmetros");
        }

        return;
    }

    parseTipoParam();  // consome tipo e param juntos

    while (currentToken.type == TOKEN_COMMA) {
        advance();
        parseTipoParam();  // aqui estava o erro
    }
}

// func ::= tipo id '(' tipos_param')' '{' { tipo decl_var{ ',' decl_var} ';' } { cmd } 
//     '}' 
//     | void id '(' tipos_param')' '{' { tipo decl_var{ ',' decl_var} ';' } { cmd 
//     } '}'
void parseFunc() {
    match(TOKEN_LBRACE);

    while (isTipo(currentToken.type)) {
        //TokenType tipoAtual = currentToken.type;
        parseTipo();
        parseDeclVarPrimeiro();
        parseDeclVarResto();
        expect(TOKEN_SEMICOLON);
    }

    while (currentToken.type != TOKEN_RBRACE && currentToken.type != TOKEN_EOF) {
        parseCmd();
    }

    match(TOKEN_RBRACE);
}


// cmd ::= if '(' expr ')' cmd [ else cmd ] 
//     | while '(' expr ')' cmd 
//     | for '(' [ atrib ] ';' [ expr ] ';' [ atrib ] ')' cmd 
//     | return [ expr ] ';' 
//     | atrib ';' 
//     | id '(' [expr { ',' expr } ] ')' ';' 
//     | '{' { cmd } '}' 
//     | ';' 
void parseCmd() {
    if (currentToken.type == TOKEN_KEYWORD_IF) {
        printf("[CMD] Reconhecido comando 'if'\n");
        advance(); // consome 'if'

        match(TOKEN_LPAREN);
        parseExpr();
        match(TOKEN_RPAREN);

        parseCmd();

        if (currentToken.type == TOKEN_KEYWORD_ELSE) {
            printf("[CMD] Reconhecido bloco 'else'\n");
            advance(); // consome 'else'
            parseCmd();
        }

    } else if (currentToken.type == TOKEN_KEYWORD_WHILE) {
        printf("[CMD] Reconhecido comando 'while'\n");
        advance(); // consome 'while'

        match(TOKEN_LPAREN);
        parseExpr();
        match(TOKEN_RPAREN);

        parseCmd();

    } else if (currentToken.type == TOKEN_KEYWORD_FOR) {
        printf("[CMD] Reconhecido comando 'for'\n");
        advance(); // consome 'for'

        match(TOKEN_LPAREN);

        // [ atrib ]
        if (currentToken.type == TOKEN_ID) {
            parseAtrib();
        }
        match(TOKEN_SEMICOLON);

        // [ expr ]
        if (currentToken.type != TOKEN_SEMICOLON) {
            parseExpr();
        }
        match(TOKEN_SEMICOLON);

        // [ atrib ]
        if (currentToken.type == TOKEN_ID) {
            parseAtrib();
        }
        match(TOKEN_RPAREN);

        parseCmd();

    } else if (currentToken.type == TOKEN_KEYWORD_RETURN) {
        printf("[CMD] Reconhecido comando 'return'\n");
        advance(); // consome 'return'

        // [ expr ]
        if (currentToken.type != TOKEN_SEMICOLON) {
            parseExpr();
        }

        match(TOKEN_SEMICOLON);

    } else if (currentToken.type == TOKEN_LBRACE) {
        printf("[CMD] Bloco composto reconhecido\n");
        advance(); // consome '{'

        while (currentToken.type != TOKEN_RBRACE && currentToken.type != TOKEN_EOF) {
            parseCmd();
        }
        match(TOKEN_RBRACE);

    } else if (currentToken.type == TOKEN_SEMICOLON) {
        printf("[CMD] Comando vazio reconhecido\n");
        advance(); // consome ';'   
    } else if (currentToken.type == TOKEN_ID) {
        Token lookahead = getNextToken(); // pega o próximo token
        ungetToken(lookahead);            // devolve ele pro fluxo

        if (lookahead.type == TOKEN_ASSIGN || lookahead.type == TOKEN_LBRACK) {
            parseAtrib();
            match(TOKEN_SEMICOLON);
            return;
        }
    }else {
        // Placeholder para comandos ainda não tratados
        printf("[CMD] Comando (placeholder) reconhecido: token '%s'\n", currentToken.lexeme);
        advance();
    }
}

// atrib ::= id [ '[' expr ']' ] = expr 
void parseAtrib() {
    if (currentToken.type != TOKEN_ID) {
        syntaxError("Esperado identificador no início da atribuição");
        return;
    }

    printf("[ATRIB] Início de atribuição: %s\n", currentToken.lexeme);
    advance();  // consome o id

    // Verifica se é uma atribuição em vetor
    if (currentToken.type == TOKEN_LBRACK) {
        printf("[ATRIB] Índice de vetor detectado\n");
        advance();  // consome '['
        parseExpr();
        match(TOKEN_RBRACK);  // consome ']'
    }

    match(TOKEN_ASSIGN);  // consome '='
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

// fator ::= id [ '[' expr ']' ] | intcon | realcon | charcon |  
//           id '(' [expr { ',' expr } ] ')'  |  '(' expr ')'  | '!' fator 
void parseFator() {
    if (currentToken.type == TOKEN_ID) {
        Token idToken = currentToken;
        advance();

        if (currentToken.type == TOKEN_LBRACK) {
            advance();
            parseExpr();
            eat(TOKEN_RBRACK);
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
            eat(TOKEN_RPAREN);
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
        eat(TOKEN_RPAREN);
    }
    else if (strcmp(currentToken.lexeme, "!") == 0) {
        advance();
        parseFator();
    }
    else {
        syntaxError("Fator inválido");
    }
}




// op_rel ::= ==  
//        |!= 
//        |<= 
//        |< 
//        |>= 
//        |> 


void eat(int expectedTokenType) {
    if (currentToken.type == expectedTokenType) {
        advance();
    } else {
        syntaxError("Token inesperado");
    }
}

void syntaxError(const char* msg) {
    fprintf(stderr, "[ERRO SINTÁTICO] %s: encontrado '%s' (linha %d, coluna %d)\n",
            msg, currentToken.lexeme, currentToken.line, currentToken.column);
    exit(EXIT_FAILURE);
}

int isTipo(TokenType t) {
    return t == TOKEN_KEYWORD_INT ||
           t == TOKEN_KEYWORD_CHAR ||
           t == TOKEN_KEYWORD_FLOAT ||
           t == TOKEN_KEYWORD_BOOL;
}

void parseDeclVarPrimeiro() {
    expect(TOKEN_ID);
    printf("[DECL_VAR] Reconhecida variável: %s\n", currentToken.lexeme);

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
        printf("[DECL_VAR] Reconhecida variável extra: %s\n", currentToken.lexeme);
        parseDeclVarPrimeiro();
    }
}

void parseTipoParam() {
    if (!isTipo(currentToken.type)) {
        erro("Esperado tipo (int, char, float, bool) no parâmetro");
    }

    advance(); // consome o tipo

    // Verifica se é '&&' (um único token do tipo TOKEN_AND)
    int porReferencia = 0;
    if (currentToken.type == TOKEN_AND) {
        porReferencia = 1;
        advance(); // consome '&&'
    }

    // Agora deve vir um identificador
    if (currentToken.type != TOKEN_ID) {
        erro("Esperado identificador no parâmetro");
    }

    char nome[256];
    strncpy(nome, currentToken.lexeme, sizeof(nome));
    nome[sizeof(nome) - 1] = '\0';
    advance(); // consome ID

    // Verifica se é vetor
    int isVetor = 0;
    if (currentToken.type == TOKEN_LBRACK) {
        advance();
        expect(TOKEN_RBRACK);
        isVetor = 1;
    }

    // Imprime o tipo de parâmetro detectado
    if (porReferencia) {
        printf("[PARAM] Parâmetro por referência: &&%s\n", nome);
    } else if (isVetor) {
        printf("[PARAM] Parâmetro vetor: %s[]\n", nome);
    } else {
        printf("[PARAM] Parâmetro comum: %s\n", nome);
    }
}

int isComandoInicio(TokenType t) {
    return t == TOKEN_KEYWORD_IF || t == TOKEN_KEYWORD_WHILE ||
           t == TOKEN_KEYWORD_RETURN || t == TOKEN_LBRACE ||
           t == TOKEN_ID || t == TOKEN_SEMICOLON;
}

void match(int expectedType) {
    if (currentToken.type == expectedType) {
        advance();
    } else {
        fprintf(stderr, "[ERRO SINTÁTICO] Esperado token do tipo %d, mas encontrado '%s' (linha %d, coluna %d)\n",
                expectedType, currentToken.lexeme, currentToken.line, currentToken.column);
        exit(EXIT_FAILURE); // encerra o programa, mas você pode adaptar para recuperação de erro
    }
}

void ungetToken(Token t) {
    backupToken = t;
    tokenBack = true;
}

// decl_var { ',' decl_var }
void parseDeclVarLista() {
    parseDeclVar(); // primeiro já consumido id

    while (currentToken.type == TOKEN_COMMA) {
        advance(); // consome ','
        parseDeclVar(); // próximo id
    }
}

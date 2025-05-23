#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "lexer.h"

// ==============================
// VARIÁVEIS INTERNAS
// ==============================

// Arquivo de entrada (código fonte)
static FILE* sourceFile = NULL;

// Contadores de linha e coluna para rastreamento de posição no código
int contLinha = 1;
static int contColuna = 1;

// Último caractere lido
static int lastChar = ' ';

// Lista de palavras-chave reconhecidas pela linguagem
#define MAX_KEYWORDS 16
const char* keywords[] = {
    "if", "else", "while", "for", "return",
    "int", "float", "char", "void", "string",
    "break", "continue", "do", "switch", "case", "default"
};
const int numKeywords = 16;

// ==============================
// FUNÇÕES AUXILIARES
// ==============================

// Verifica se um lexema corresponde a uma palavra-chave
int isKeyword(const char* lexeme) {
    for (int i = 0; i < numKeywords; i++) {
        if (strcmp(lexeme, keywords[i]) == 0) return 1;
    }
    return 0;
}

// Cria um token com as informações apropriadas
Token makeToken(TokenType type, const char* lexeme, int line, int col) {
    Token t;
    strncpy(t.lexeme, lexeme, TAM_MAX_LEXEMA - 1);
    t.lexeme[TAM_MAX_LEXEMA - 1] = '\0';
    t.line = line;
    t.column = col;
    t.type = type;

    // Preenche os campos específicos do token
    if (type == TOKEN_INTCON)
        t.intVal = atoi(lexeme);
    else if (type == TOKEN_REALCON)
        t.realVal = atof(lexeme);
    else if (type == TOKEN_CHARCON)
        t.charVal = lexeme[0];
    else
        t.strVal = strdup(lexeme); // para strings e identificadores

    return t;
}

// Lê o próximo caractere do arquivo fonte e atualiza contadores
int nextChar() {
    int c = fgetc(sourceFile);
    if (c == '\n') {
        contLinha++;
        contColuna = 1;
    } else {
        contColuna++;
    }
    return c;
}

// Ignora espaços em branco, tabs e novas linhas
void skipWhitespace() {
    while (isspace(lastChar)) {
        lastChar = nextChar();
    }
}

// ==============================
// INTERFACE PÚBLICA
// ==============================

// Inicializa o analisador léxico com o arquivo fonte
void initLexer(FILE* source) {
    sourceFile = source;
    contLinha = 1;
    contColuna = 1;
    lastChar = nextChar(); // pega o primeiro caractere
}

// Finaliza o analisador léxico
void destroyLexer() {
    sourceFile = NULL;
}

// ==============================
// FUNÇÃO PRINCIPAL DO ANALISADOR
// ==============================

// Retorna o próximo token do código-fonte
Token getNextToken() {
    skipWhitespace();

    int line = contLinha;
    int col = contColuna;
    char lexeme[TAM_MAX_LEXEMA];
    int i = 0;

    // Fim de arquivo
    if (lastChar == EOF) {
        return makeToken(TOKEN_EOF, "EOF", line, col);
    }

    // Identificadores e palavras-chave
    if (isalpha(lastChar) || lastChar == '_') {
        while (isalnum(lastChar) || lastChar == '_') {
            if (i < TAM_MAX_LEXEMA - 1)
                lexeme[i++] = lastChar;
            lastChar = nextChar();
        }
        lexeme[i] = '\0';
        if (isKeyword(lexeme))
            return makeToken(TOKEN_KEYWORD, lexeme, line, col);
        return makeToken(TOKEN_ID, lexeme, line, col);
    }

    // Constantes numéricas (inteiras ou reais)
    if (isdigit(lastChar)) {
        int isReal = 0;
        while (isdigit(lastChar) || lastChar == '.') {
            if (lastChar == '.') isReal = 1;
            if (i < TAM_MAX_LEXEMA - 1)
                lexeme[i++] = lastChar;
            lastChar = nextChar();
        }
        lexeme[i] = '\0';
        return makeToken(isReal ? TOKEN_REALCON : TOKEN_INTCON, lexeme, line, col);
    }

    // Constantes de caractere (ex: 'a')
    if (lastChar == '\'') {
        lexeme[i++] = lastChar;
        lastChar = nextChar();
        if (lastChar == '\\') { // caractere de escape
            lexeme[i++] = lastChar;
            lastChar = nextChar();
        }
        lexeme[i++] = lastChar;
        lastChar = nextChar();
        if (lastChar == '\'') {
            lexeme[i++] = lastChar;
            lexeme[i] = '\0';
            lastChar = nextChar();
            return makeToken(TOKEN_CHARCON, lexeme + 1, line, col); // ignora aspas
        } else {
            return makeToken(TOKEN_INVALID, "Invalid char", line, col);
        }
    }

    // Constantes de string (ex: "texto")
    if (lastChar == '"') {
        lastChar = nextChar();
        while (lastChar != '"' && lastChar != EOF && i < TAM_MAX_LEXEMA - 1) {
            lexeme[i++] = lastChar;
            lastChar = nextChar();
        }
        if (lastChar == '"') {
            lexeme[i] = '\0';
            lastChar = nextChar();
            return makeToken(TOKEN_STRINGCON, lexeme, line, col);
        } else {
            return makeToken(TOKEN_INVALID, "Unclosed string", line, col);
        }
    }

    // Comentários e operador de divisão
    if (lastChar == '/') {
        lastChar = nextChar();
        if (lastChar == '/') {
            // Comentário de linha
            while (lastChar != '\n' && lastChar != EOF) {
                lastChar = nextChar();
            }
            return getNextToken(); // ignora e volta ao início
        } else if (lastChar == '*') {
            // Comentário de bloco
            int state = 0;
            while (state != 2 && lastChar != EOF) {
                lastChar = nextChar();
                if (lastChar == '*') state = 1;
                else if (lastChar == '/' && state == 1) state = 2;
                else state = 0;
            }
            lastChar = nextChar(); // sai do comentário
            return getNextToken();
        } else {
            return makeToken(TOKEN_DIV, "/", line, col);
        }
    }

    // Operadores e delimitadores
    switch (lastChar) {
        case '+': lastChar = nextChar(); return makeToken(TOKEN_PLUS, "+", line, col);
        case '-': lastChar = nextChar(); return makeToken(TOKEN_MINUS, "-", line, col);
        case '*': lastChar = nextChar(); return makeToken(TOKEN_MUL, "*", line, col);
        case '=':
            lastChar = nextChar();
            if (lastChar == '=') {
                lastChar = nextChar();
                return makeToken(TOKEN_EQ, "==", line, col);
            }
            return makeToken(TOKEN_ASSIGN, "=", line, col);
        case '!':
            lastChar = nextChar();
            if (lastChar == '=') {
                lastChar = nextChar();
                return makeToken(TOKEN_NEQ, "!=", line, col);
            }
            return makeToken(TOKEN_INVALID, "!", line, col);
        case '<':
            lastChar = nextChar();
            if (lastChar == '=') {
                lastChar = nextChar();
                return makeToken(TOKEN_LEQ, "<=", line, col);
            }
            return makeToken(TOKEN_LT, "<", line, col);
        case '>':
            lastChar = nextChar();
            if (lastChar == '=') {
                lastChar = nextChar();
                return makeToken(TOKEN_GEQ, ">=", line, col);
            }
            return makeToken(TOKEN_GT, ">", line, col);
        case '&':
            lastChar = nextChar();
            if (lastChar == '&') {
                lastChar = nextChar();
                return makeToken(TOKEN_AND, "&&", line, col);
            }
            return makeToken(TOKEN_INVALID, "&", line, col);
        case '(': lastChar = nextChar(); return makeToken(TOKEN_LPAREN, "(", line, col);
        case ')': lastChar = nextChar(); return makeToken(TOKEN_RPAREN, ")", line, col);
        case '[': lastChar = nextChar(); return makeToken(TOKEN_LBRACK, "[", line, col);
        case ']': lastChar = nextChar(); return makeToken(TOKEN_RBRACK, "]", line, col);
        case '{': lastChar = nextChar(); return makeToken(TOKEN_LBRACE, "{", line, col);
        case '}': lastChar = nextChar(); return makeToken(TOKEN_RBRACE, "}", line, col);
        case ';': lastChar = nextChar(); return makeToken(TOKEN_SEMICOLON, ";", line, col);
        case ',': lastChar = nextChar(); return makeToken(TOKEN_COMMA, ",", line, col);
        default:
            // Caractere não reconhecido
            lexeme[0] = lastChar;
            lexeme[1] = '\0';
            lastChar = nextChar();
            return makeToken(TOKEN_INVALID, lexeme, line, col);
    }


}

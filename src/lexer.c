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

const char* tokenTypeName(TokenType type) {
    switch (type) {
        case TOKEN_ID: return "id";
        case TOKEN_INTCON: return "intcon";
        case TOKEN_REALCON: return "realcon";
        case TOKEN_CHARCON: return "charcon";
        case TOKEN_CHARCON_N: return "charcon_n";
        case TOKEN_CHARCON_0: return "charcon_0";
        case TOKEN_STRINGCON: return "stringcon";
        case TOKEN_PLUS: return "+";
        case TOKEN_MINUS: return "-";
        case TOKEN_MUL: return "*";
        case TOKEN_DIV: return "/";
        case TOKEN_LT: return "<";
        case TOKEN_LEQ: return "<=";
        case TOKEN_GT: return ">";
        case TOKEN_GEQ: return ">=";
        case TOKEN_EQ: return "==";
        case TOKEN_NEQ: return "!=";
        case TOKEN_ASSIGN: return "=";
        case TOKEN_BITAND: return "&";
        case TOKEN_AND: return "&&";
        case TOKEN_OR: return "||";
        case TOKEN_LPAREN: return "(";
        case TOKEN_RPAREN: return ")";
        case TOKEN_LBRACK: return "[";
        case TOKEN_RBRACK: return "]";
        case TOKEN_LBRACE: return "{";
        case TOKEN_RBRACE: return "}";
        case TOKEN_SEMICOLON: return ";";
        case TOKEN_COMMA: return ",";
        case TOKEN_KEYWORD: return "palavra-chave";
        case TOKEN_EOF: return "EOF";
        case TOKEN_INVALID: return "inválido";
        default: return "desconhecido";
    }
}

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
        if (strcmp(lexeme, "int") == 0)       return makeToken(TOKEN_KEYWORD_INT, lexeme, line, col);
        else if (strcmp(lexeme, "char") == 0) return makeToken(TOKEN_KEYWORD_CHAR, lexeme, line, col);
        else if (strcmp(lexeme, "bool") == 0) return makeToken(TOKEN_KEYWORD_BOOL, lexeme, line, col);
        else if (strcmp(lexeme, "float") == 0) return makeToken(TOKEN_KEYWORD_FLOAT, lexeme, line, col);
        else if (strcmp(lexeme, "if") == 0)   return makeToken(TOKEN_KEYWORD_IF, lexeme, line, col);
        else if (strcmp(lexeme, "else") == 0) return makeToken(TOKEN_KEYWORD_ELSE, lexeme, line, col);
        else if (strcmp(lexeme, "while") == 0) return makeToken(TOKEN_KEYWORD_WHILE, lexeme, line, col);
        else if (strcmp(lexeme, "for") == 0)  return makeToken(TOKEN_KEYWORD_FOR, lexeme, line, col);
        else if (strcmp(lexeme, "return") == 0) return makeToken(TOKEN_KEYWORD_RETURN, lexeme, line, col);
        else if (strcmp(lexeme, "void") == 0) return makeToken(TOKEN_KEYWORD_VOID, lexeme, line, col);
        else if (strcmp(lexeme, "break") == 0) return makeToken(TOKEN_KEYWORD_BREAK, lexeme, line, col);
        else if (strcmp(lexeme, "continue") == 0) return makeToken(TOKEN_KEYWORD_CONTINUE, lexeme, line, col);
        else if (strcmp(lexeme, "do") == 0) return makeToken(TOKEN_KEYWORD_DO, lexeme, line, col);
        else if (strcmp(lexeme, "switch") == 0) return makeToken(TOKEN_KEYWORD_SWITCH, lexeme, line, col);
        else if (strcmp(lexeme, "case") == 0) return makeToken(TOKEN_KEYWORD_CASE, lexeme, line, col);
        else if (strcmp(lexeme, "default") == 0) return makeToken(TOKEN_KEYWORD_DEFAULT, lexeme, line, col);
        else if (strcmp(lexeme, "string") == 0) return makeToken(TOKEN_KEYWORD_STRING, lexeme, line, col);
        else return makeToken(TOKEN_ID, lexeme, line, col);
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

    // Constantes de caractere (ex: 'a', '\n', '\0')
    if (lastChar == '\'') {
        int startCol = col; // salva coluna para diagnóstico se der erro
        lexeme[0] = '\''; // abre aspas

        lastChar = nextChar();

        TokenType charTokenType;

        if (lastChar == '\\') {
            char escapedChar = nextChar();
            lexeme[1] = '\\';
            lexeme[2] = escapedChar;
            lexeme[3] = '\'';  // fecha aspas
            lexeme[4] = '\0';

            if (escapedChar == 'n') {
                charTokenType = TOKEN_CHARCON_N;
            } else if (escapedChar == '0') {
                charTokenType = TOKEN_CHARCON_0;
            } else {
                charTokenType = TOKEN_CHARCON; // outros escapes como \t, \r, etc.
            }

            lastChar = nextChar(); // consome a aspa final
        } else {
            lexeme[1] = lastChar;
            lexeme[2] = '\'';
            lexeme[3] = '\0';

            charTokenType = TOKEN_CHARCON;
            lastChar = nextChar(); // consome a aspa final
        }

        if (lastChar == '\'') {
            lastChar = nextChar(); // consome o próximo caractere após a aspa
            return makeToken(charTokenType, lexeme, line, startCol);
        } else {
            return makeToken(TOKEN_INVALID, "Invalid char", line, startCol);
        }
    }

    // Constantes de string (ex: "texto")
    if (lastChar == '"') {
        lexeme[i++] = '"'; // adiciona a aspa de abertura
        lastChar = nextChar();
        while (lastChar != '"' && lastChar != EOF && i < TAM_MAX_LEXEMA - 2) {
            lexeme[i++] = lastChar;
            lastChar = nextChar();
        }
        if (lastChar == '"') {
            lexeme[i++] = '"'; // adiciona a aspa de fechamento
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
            return makeToken(TOKEN_BITAND, "&", line, col);
        case '|':
            lastChar = nextChar();
            if (lastChar == '|') {
                lastChar = nextChar();
                return makeToken(TOKEN_OR, "||", line, col);
            }
            return makeToken(TOKEN_INVALID, "|", line, col);
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
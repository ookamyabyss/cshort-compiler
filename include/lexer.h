#ifndef LEXER_H
#define LEXER_H

#include <stdio.h>

#define TAM_MAX_LEXEMA 128

// Tipos de tokens reconhecidos
typedef enum {
    TOKEN_ID,          // Identificador
    TOKEN_INTCON,      // Constante inteira
    TOKEN_REALCON,     // Constante real
    TOKEN_CHARCON,     // Constante de caractere comum ex: 'a'
    TOKEN_CHARCON_N,   // Constante de caractere '\n'
    TOKEN_CHARCON_0,   // Constante de caractere '\0'
    TOKEN_STRINGCON,   // Constante string
    TOKEN_COMMENT,     // Comentário

    // Operadores aritméticos
    TOKEN_PLUS,       // +
    TOKEN_MINUS,      // -
    TOKEN_MUL,        // *
    TOKEN_DIV,        // /

    // Operadores relacionais e lógicos
    TOKEN_EQ,         // ==
    TOKEN_NEQ,        // !=
    TOKEN_LT,         // <
    TOKEN_GT,         // >
    TOKEN_LEQ,        // <=
    TOKEN_GEQ,        // >=
    TOKEN_ASSIGN,     // =
    TOKEN_AND,        // &&

    // Delimitadores
    TOKEN_LPAREN,     // (
    TOKEN_RPAREN,     // )
    TOKEN_LBRACK,     // [
    TOKEN_RBRACK,     // ]
    TOKEN_LBRACE,     // {
    TOKEN_RBRACE,     // }
    TOKEN_SEMICOLON,  // ;
    TOKEN_COMMA,      // ,

    // Palavra-chave (verificada depois)
    TOKEN_KEYWORD,

    // Outros
    TOKEN_EOF,
    TOKEN_INVALID
} TokenType;

// Subcategorias opcionais
typedef enum {
    OP_PLUS,
    OP_MINUS,
    OP_MUL,
    OP_DIV,
    OP_ASSIGN,
    OP_EQ,
    OP_NEQ,
    OP_LT,
    OP_GT,
    OP_LEQ,
    OP_GEQ,
    OP_AND
} OperatorType;

// Estrutura principal de um token
typedef struct {
    TokenType type;       // Tipo principal do token

    union {
        int intVal;       // Se TOKEN_INTCON
        float realVal;    // Se TOKEN_REALCON
        char charVal;     // Se TOKEN_CHARCON
        char* strVal;     // Se TOKEN_STRINGCON ou TOKEN_ID ou palavra-chave
    };

    char lexeme[TAM_MAX_LEXEMA]; // Lexema original (útil sempre)
    int line;           // Linha de origem
    int column;         // Coluna inicial
} Token;

// Variável global para controle de linha 
extern int contLinha;

// Funções do analisador léxico
void initLexer(FILE* source);   // Inicializa com um arquivo fonte
Token getNextToken();           // Retorna próximo token
void destroyLexer();            // Libera recursos

const char* tokenTypeName(TokenType type);

#endif // LEXER_H

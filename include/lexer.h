#ifndef LEXER_H
#define LEXER_H

#include <stdio.h>

#define TAM_MAX_LEXEMA 128

// Tipos primitivos de token literal usados internamente
#define TOKEN_INT_LITERAL 301
#define TOKEN_CHAR_LITERAL 302

// Enumeração de todos os tipos de tokens reconhecidos
typedef enum {
    // Identificadores e literais
    TOKEN_ID,              // Identificador
    TOKEN_INTCON,          // Constante inteira (ex: 10)
    TOKEN_REALCON,         // Constante real (ex: 3.14)
    TOKEN_CHARCON,         // Constante de caractere comum (ex: 'a')
    TOKEN_CHARCON_N,       // Constante de caractere especial '\n'
    TOKEN_CHARCON_0,       // Constante de caractere nulo '\0'
    TOKEN_BOOLCON,         // Constantes booleanas: true / false
    TOKEN_STRINGCON,       // Constante string

    // Operadores aritméticos e lógicos
    TOKEN_PLUS,            // +
    TOKEN_MINUS,           // -
    TOKEN_MUL,             // *
    TOKEN_DIV,             // /
    TOKEN_NOT,             // !

    // Operadores relacionais e de atribuição
    TOKEN_EQ,              // ==
    TOKEN_NEQ,             // !=
    TOKEN_LT,              // <
    TOKEN_GT,              // >
    TOKEN_LEQ,             // <=
    TOKEN_GEQ,             // >=
    TOKEN_ASSIGN,          // =

    // Operadores lógicos binários
    TOKEN_BITAND,           // &
    TOKEN_AND,             // &&
    TOKEN_OR,              // ||

    // Delimitadores
    TOKEN_LPAREN,          // (
    TOKEN_RPAREN,          // )
    TOKEN_LBRACK,          // [
    TOKEN_RBRACK,          // ]
    TOKEN_LBRACE,          // {
    TOKEN_RBRACE,          // }
    TOKEN_SEMICOLON,       // ;
    TOKEN_COMMA,           // ,

    // Palavras-chave da linguagem C.Short
    TOKEN_KEYWORD_INT,
    TOKEN_KEYWORD_CHAR,
    TOKEN_KEYWORD_FLOAT,
    TOKEN_KEYWORD_BOOL,
    TOKEN_KEYWORD_IF,
    TOKEN_KEYWORD_ELSE,
    TOKEN_KEYWORD_WHILE,
    TOKEN_KEYWORD_RETURN,
    TOKEN_KEYWORD_FOR,
    TOKEN_KEYWORD_VOID,

    // Outros
    TOKEN_EOF,             // Fim de arquivo
    TOKEN_INVALID          // Token inválido (erro léxico)
} TokenType;

// Estrutura que representa um token identificado pelo lexer
typedef struct {
    TokenType type;                 // Tipo do token

    union {
        int intVal;                // Valor inteiro (se TOKEN_INTCON)
        float realVal;             // Valor real (se TOKEN_REALCON)
        char charVal;              // Valor char (se TOKEN_CHARCON)
        char* strVal;              // Valor string (ID, palavras-chave, strings)
    };

    char lexeme[TAM_MAX_LEXEMA];   // Lexema original como aparece no código
    int line;                      // Linha em que o token aparece
    int column;                    // Coluna inicial do token
} Token;

// Variável global de controle de linha (atualizada pelo lexer)
extern int contLinha;

// Funções principais do analisador léxico
void initLexer(FILE* source);        // Inicializa o lexer com um arquivo fonte
Token getNextToken();                // Retorna o próximo token do código-fonte
void destroyLexer();                 // Libera memória ou buffers usados pelo lexer

// Utilitário para obter o nome textual de um tipo de token
const char* tokenTypeName(TokenType type);

#endif 
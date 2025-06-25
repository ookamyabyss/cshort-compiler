#ifndef PARSER_H
#define PARSER_H

#include <stdio.h>
#include "lexer.h"
#include "symbols.h"


// ==============================
// Inicialização
// ==============================

/**
 * Inicia o analisador sintático com o arquivo fonte.
 */
void startParser(FILE* f);

// ==============================
// Regras da gramática principal
// ==============================

void parseProg(void);         // prog ::= { decl ';' | func }
void parseDecl(void);         // decl ::= tipo decl_var {...} | tipo id(...) {...} | void id(...) {...}
void parseDeclVar(const char* tipo, Escopo escopo);      // decl_var ::= id [ '[' intcon ']' ]
void parseTipo(void);         // tipo ::= char | int | float | bool
void parseTiposParam(void);   // tipos_param ::= void | tipo (id | &id | id[]){, tipo (...)}

void parseFunc(void);         // func ::= tipo/void id(...) '{' {decl_var} {cmd} '}' 
void parseCmd(void);          // cmd ::= if, while, for, return, atrib, chamada, bloco, ';'
void parseAtrib(void);        // atrib ::= id [ '[' expr ']' ] = expr

void parseExpr(void);         // expr ::= expr_simp [ op_rel expr_simp ]
void parseExprSimp(void);     // expr_simp ::= [+|-] termo {(+|-|or) termo}
void parseTermo(void);        // termo ::= fator {(*|/|and) fator}
void parseFator(void);        // fator ::= id[...] | constantes | chamada | (!fator)

// ==============================
// Funções auxiliares de análise
// ==============================

void parseTipoParam(void);            // tipo (id | &id | id[])
void parseDeclVarPrimeiro(const char* tipo, Escopo escopo);       // primeira variável da lista
void parseDeclVarResto(const char* tipo, Escopo escopo);         // demais variáveis após vírgula
void parseDeclVarLista(const char* tipo, Escopo escopo);        // lista de variáveis tipo v1, v2, v3;

// ==============================
// Utilitários de parsing
// ==============================

int isTipo(TokenType t);              // verifica se t é tipo válido
int isComandoInicio(TokenType t);     // verifica se t inicia comando

void eat(int expectedTokenType);      // consome token esperado, erro se não for
void match(int expectedType);         // consome token, erro se diferente
void syntaxError(const char* msg);    // exibe erro sintático com mensagem

void ungetToken(Token t);             // "devolve" token ao fluxo léxico




#endif // PARSER_H
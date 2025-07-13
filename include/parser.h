#ifndef PARSER_H
#define PARSER_H

#include <stdio.h>
#include "lexer.h"
#include "symbols.h"

#define MAX_PARAMS_FUNCAO 32

extern char tiposParamsTemp[MAX_PARAMS_FUNCAO][10];
extern char nomesParamsTemp[MAX_PARAMS_FUNCAO][256];
extern int numParamsTemp;

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

void ungetToken(Token t);             // "devolve" token ao fluxo léxico

void parseEat(int expectedType);     // consome token, erro se diferente

// Retorna em 'dest' o nome do tipo correspondente ao token atual
void obterTipoString(char* dest); 

#endif // PARSER_H
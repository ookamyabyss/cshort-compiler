#ifndef PARSER_H
#define PARSER_H

#include <stdio.h>
#include "lexer.h"

// Inicializa o parser com o arquivo fonte
void startParser(FILE* f);

// Função principal que inicia a análise sintática (ponto de entrada: prog)

// prog ::= { decl ';' | func }
void parseProg();

// decl ::= tipo decl_var { ',' decl_var} 
//      | tipo id '(' tipos_param')' { ',' id '(' tipos_param')' } 
//      | void id '(' tipos_param')' { ',' id '(' tipos_param')' }
void parseDecl();

// decl_var ::= id [ '[' intcon ']' ]
void parseDeclVar();

// tipo ::= char | int | float | bool
void parseTipo();

//tipos_param ::= void 
//              | tipo (id | &&id | id '[' ']') { ','  tipo (id | &&id | id '[' ']') }
void parseTiposParam();

// func ::= tipo id '(' tipos_param')' '{' { tipo decl_var{ ',' decl_var} ';' } { cmd } 
//     '}' 
//     | void id '(' tipos_param')' '{' { tipo decl_var{ ',' decl_var} ';' } { cmd 
//     } '}'  
void parseFunc();

// cmd ::= if '(' expr ')' cmd [ else cmd ] 
//     | while '(' expr ')' cmd 
//     | for '(' [ atrib ] ';' [ expr ] ';' [ atrib ] ')' cmd 
//     | return [ expr ] ';' 
//     | atrib ';' 
//     | id '(' [expr { ',' expr } ] ')' ';' 
//     | '{' { cmd } '}' 
//     | ';' 
void parseCmd();

// atrib ::= id [ '[' expr ']' ] = expr 
void parseAtrib();

// expr ::= expr_simp [ op_rel  expr_simp ] 
void parseExpr();

// expr_simp ::= [+ | – ] termo {(+ | – | ||) termo} 
void parseExprSimp();

// termo ::= fator {(* | / | &&)  fator} 
void parseTermo();

// fator ::= id [ '[' expr ']' ] | intcon | realcon | charcon |  
//           id '(' [expr { ',' expr } ] ')'  |  '(' expr ')'  | '!' fator 
void parseFator();

void parseTipoParamVar();
int isTipo(TokenType t);
void parseParam();
void parseDeclVarResto();
void parseDeclVarPrimeiro();
void parseTipoParam();
int isComandoInicio(TokenType t);

// Prototypes das funções usadas

void eat(int expectedTokenType);
void syntaxError(const char* msg);

void match(int expectedType);

void parseAtribComIDJaLido();

void ungetToken(Token tok);

void parseLocalDecl();
#endif // PARSER_H

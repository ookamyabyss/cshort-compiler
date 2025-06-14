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

void parseTipo();

void parseTiposParam();
void parseTipoParamVar();
int isTipo(TokenType t);  
void parseFunc();
void parseCmd();
void parseParam();
void parseDeclVarResto();

void parseDeclVarPrimeiro();


#endif // PARSER_H

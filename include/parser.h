#ifndef PARSER_H
#define PARSER_H

#include <stdio.h>
#include "lexer.h"

// Inicializa o parser com o arquivo fonte
void startParser(FILE* f);

// Função principal que inicia a análise sintática (ponto de entrada: prog)
void parseProg();

void parseDecl();
void parseTipo();
void parseDeclVar();
void parseDeclVarResto();
void parseTiposParam();
void parseTipoParamVar();
int isTipo(TokenType t);  
void parseFunc();
void parseCmd();
void parseParam();


#endif // PARSER_H

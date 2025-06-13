#ifndef PARSER_H
#define PARSER_H
#include <stdio.h>

// Inicializa o parser com o arquivo fonte
void startParser(FILE* f);

// Função principal que inicia a análise sintática (ponto de entrada: prog)
void parseProg();

#endif

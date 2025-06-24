#ifndef SYMBOLS_H
#define SYMBOLS_H

#define MAX_TABELA 1000

// Escopo possível de um símbolo
typedef enum {
    ESC_GLOBAL,
    ESC_LOCAL
} Escopo;

// Classe possível de um símbolo
typedef enum {
    CLASSE_VAR,       // variável
    CLASSE_VETOR,     // vetor
    CLASSE_FUNCAO,    // função
    CLASSE_PARAM      // parâmetro (normal, por ref, ou vetor)
} Classe;

// Representa um símbolo na tabela
typedef struct {
    char nome[64];     // identificador (id)
    char tipo[10];     // "int", "float", "char", "bool", "void"
    Classe classe;     // tipo de símbolo
    Escopo escopo;     // global ou local
    int tamanho;       // 1 (var), >1 (vetor), 0 (função ou parâmetro por referência)
} Simbolo;

// Interface pública da tabela de símbolos
void inicializarTabela();
int inserirSimbolo(const char* nome, const char* tipo, Classe classe, Escopo escopo, int tamanho);
Simbolo* buscarSimbolo(const char* nome, Escopo escopo);
void limparEscopo(Escopo escopo);  // remove todos os símbolos com escopo local
void imprimirTabela();             // debug: imprime todos os símbolos

// Função utilitária para parser registrar variáveis globais
void registrarVariavelGlobal(const char* tipo, const char* nome, int isVetor, int tamanho);

void registrarFuncao(const char* tipo, const char* nome);

#endif

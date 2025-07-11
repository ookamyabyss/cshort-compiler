#ifndef SYMBOLS_H

#define SYMBOLS_H
#define MAX_TABELA 1000
#define MAX_SIMBOLOS 1024

// Escopo possível de um símbolo (global ou local)
typedef enum {
    ESC_GLOBAL,
    ESC_LOCAL
} Escopo;

// Classe possível de um símbolo na tabela (var, vetor, função, parâmetro)
typedef enum {
    CLASSE_VAR,       // variável simples
    CLASSE_VETOR,     // vetor declarado com colchetes
    CLASSE_FUNCAO,    // declaração de função
    CLASSE_PARAM      // parâmetro (normal, por referência ou vetor)  
} Classe;

// Estado de validade do símbolo (se ainda está "vivo" no escopo)
typedef enum {
    ESTADO_VIVO,  // Símbolo ativo e acessível
    ESTADO_ZUMBI  // Símbolo fora de escopo, inacessível
} Estado;

// Estrutura que representa uma entrada na tabela de símbolos
typedef struct {
    char nome[64];     // identificador (nome da variável, função, etc.)
    char tipo[10];     // tipo associado: "int", "float", "char", "bool", "void"
    Classe classe;     // tipo de entidade (variável, função, etc.)
    Escopo escopo;     // escopo onde foi declarado (global/local)
    Estado estado;     // se ainda pode ser usado (vivo ou zumbi)
    int tamanho;       // tamanho usado em vetores; 1 para var simples; 0 para função/ref
} Simbolo;

// Array com todos os símbolos registrados
extern Simbolo tabelaSimbolos[MAX_SIMBOLOS];

// Número atual de símbolos registrados na tabela
extern int numSimbolos;

// Escopo atual do compilador (global ou local)
extern Escopo escopoAtual;

// ===== Interface pública da tabela de símbolos =====

// Inicializa a tabela de símbolos (zera tudo)
void inicializarTabela();

// Insere um novo símbolo na tabela, retorna índice ou erro
int inserirSimbolo(const char* nome, const char* tipo, Classe classe, Escopo escopo, int tamanho);

// Busca um símbolo com nome e escopo exatos
Simbolo* buscarSimbolo(const char* nome, Escopo escopo);

// Remove todos os símbolos do escopo fornecido (usado para limpar escopo local)
void limparEscopo(Escopo escopo);

// Imprime a tabela de símbolos atual (para debug)
void imprimirTabela();

// ===== Funções auxiliares chamadas pelo parser =====

// Registra uma variável global (tipo, nome, se é vetor e tamanho)
void registrarVariavelGlobal(const char* tipo, const char* nome, int isVetor, int tamanho);

// Registra uma nova função na tabela de símbolos
void registrarFuncao(const char* tipo, const char* nome);

// Registra um parâmetro de função (normal, por ref, ou vetor)
void registrarParametro(const char* tipo, const char* nome, Classe classe);

// Registra uma variável local (tipo, nome, se é vetor e tamanho)
void registrarVariavelLocal(const char* tipo, const char* nome, int isVetor, int tamanho);

// Busca um símbolo nos escopos disponíveis (primeiro local, depois global)
Simbolo* buscarSimboloEmEscopos(const char* nome);

#endif
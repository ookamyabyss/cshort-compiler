#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include "codegen.h"

// ============================
// CONTADORES INTERNOS
// ============================

// Contadores internos para geração de nomes únicos de temporários e labels
static int contadorTemporarios = 0;
static int contadorLabels = 0;
static FILE* arquivoSaida = NULL;

static char ultimoResultado[64];  // Guarda o nome do último temporário usado

// ============================
// INICIALIZAÇÃO / FINALIZAÇÃO
// ============================

// Abre o arquivo onde o código intermediário será escrito
void iniciarCodegen(const char* nomeArquivo) {
    arquivoSaida = fopen(nomeArquivo, "w");
    if (!arquivoSaida) {
        fprintf(stderr, "[ERRO] Não foi possível criar arquivo de saída: %s\n", nomeArquivo);
        exit(1);
    }
    fprintf(stdout, "[CODEGEN] Arquivo de saída aberto: %s\n", nomeArquivo);
}

// Fecha o arquivo de código intermediário
void finalizarCodegen() {
    if (arquivoSaida) {
        fclose(arquivoSaida);
        arquivoSaida = NULL;
        fprintf(stdout, "[CODEGEN] Arquivo de saída fechado com sucesso.\n");
    } else {
        fprintf(stderr, "[ERRO] finalizarCodegen: Nenhum arquivo estava aberto.\n");
    }
}

// ============================
// TEMPORÁRIOS E LABELS
// ============================

// Gera um novo nome de temporário único (ex: T0, T1...)
char* novoTemporario() {
    char* temp = (char*)malloc(10);  // Ex: "T0"
    if (!temp) {
        fprintf(stderr, "[ERRO] Falha ao alocar temporário\n");
        exit(1);
    }
    sprintf(temp, "T%d", contadorTemporarios++);
    return temp;
}

// Gera um novo nome de label único (ex: LABEL0, LABEL1...)
char* novaLabel() {
    char* label = malloc(16);  // Ex: "LABEL0"
    if (!label) {
        fprintf(stderr, "[ERRO] Falha ao alocar label\n");
        exit(1);
    }
    sprintf(label, "LABEL %d", contadorLabels++);
    return label;
}

// ============================
// FUNÇÕES DE BAIXO NÍVEL
// ============================

// Escreve um comando formatado no arquivo de saída
void gerarComando(const char* formato, ...) {
    if (!arquivoSaida) return;

    va_list args;
    va_start(args, formato);
    vfprintf(arquivoSaida, formato, args);
    fprintf(arquivoSaida, "\n");
    va_end(args);
}

// ============================
// FUNÇÕES DE COMANDO DIRETO
// ============================

// Gera um comando LOAD (carrega valor para o topo da pilha)
void gerarLoad(const char* varOuTemp) {
    gerarComando("LOAD %s", varOuTemp);
}

// Gera um comando STORE (guarda o topo da pilha na variável)
void gerarStore(const char* varOuTemp) {
    gerarComando("STORE %s", varOuTemp);
}

// Simula um store em vetor: var[index] = valor
void gerarStoreArray(const char* var, int tempIndex, int tempValor) {
    printf("STORE %s[T%d], T%d\n", var, tempIndex, tempValor);
}

// Empilha valor diretamente
void gerarPush(const char* valor) {
    gerarComando("PUSH %s", valor);
}

// Operações aritméticas básicas
void gerarAdd() {
    gerarComando("ADD");
}

void gerarSub() {
    gerarComando("SUB");
}

// Desvios incondicionais e condicionais
void gerarGoto(const char* label) {
    gerarComando("GOTO %s", label);
}

void gerarGotoTrue(const char* label) {
    gerarComando("GOTRUE %s", label);
}

void gerarGotoFalse(const char* label) {
    gerarComando("GOFALSE %s", label);
}

// Define um label no código
void gerarLabel(const char* label) {
    gerarComando("%s:", label);
}

// ============================
// FUNÇÕES DE BLOCO
// ============================

// Início de uma função
void gerarInicioFuncao(const char* nomeFunc) {
    gerarComando("FUNC %s:", nomeFunc);
}

// Fim de uma função
void gerarFimFuncao() {
    gerarComando("ENDFUNC");
}

// Declaração de variável (ex: VAR x : int)
void gerarDeclaracaoVar(const char* nomeVar, const char* tipo) {
    gerarComando("VAR %s : %s", nomeVar, tipo);
}

// Gera um comando vazio (;)
void gerarComandoVazio() {
    gerarComando(";" );
}

// Gera LOAD + STORE para uma atribuição simples
void gerarRetorno(const char* expr) {
    if (expr && strlen(expr) > 0)
        gerarComando("RETURN %s", expr);
    else
        gerarComando("RETURN");
}

// Gera LOAD + STORE para uma atribuição simples
void gerarAtribuicao(const char* destino, const char* fonte) {
    gerarComando("LOAD %s", fonte);
    gerarComando("STORE %s", destino);
}

// ============================
// COMPARAÇÕES RELACIONAIS
// ============================

// Gera código para comparações relacionais (==, !=, >, <, >=, <=)
void gerarComparacaoRelacional(const char* op, const char* esq, const char* dir, const char* destino) {
    gerarLoad(esq);
    gerarLoad(dir);

    if (strcmp(op, "==") == 0) {
        gerarComando("EQ");
    } else if (strcmp(op, "!=") == 0) {
        gerarComando("NE");
    } else if (strcmp(op, ">") == 0) {
        gerarComando("GT");
    } else if (strcmp(op, ">=") == 0) {
        gerarComando("GE");
    } else if (strcmp(op, "<") == 0) {
        gerarComando("LT");
    } else if (strcmp(op, "<=") == 0) {
        gerarComando("LE");
    } else {
        fprintf(stderr, "[ERRO] Operador relacional desconhecido: %s\n", op);
        gerarComando("ERRO_OP_REL");
    }

    gerarStore(destino);
}

// ============================
// CONTROLE DE FLUXO (IF, WHILE, FOR)
// ============================

// Desvio para label se condição for falsa
void gerarIfFalso(const char* condicao, const char* labelFalsa) {
    gerarComando("GOFALSE %s", labelFalsa);
}

// Desvio para label se condição for verdadeira
void gerarIf(const char* condicao, const char* labelVerdadeira) {
    gerarComando("GOTRUE %s", labelVerdadeira);
}

// Traduz if-else completo com blocos de código
void gerarIfElseComBlocos(const char* condicao, void (*blocoIf)(void), void (*blocoElse)(void)) {
    char* labelElse = novaLabel();
    char* labelFim = novaLabel();

    gerarGotoFalse(labelElse);

    if (blocoIf) blocoIf();

    gerarGoto(labelFim);
    gerarLabel(labelElse);

    if (blocoElse) blocoElse();

    gerarLabel(labelFim);

    free(labelElse);
    free(labelFim);
}

// Gera estrutura de repetição while (com corpo e condição)
void gerarWhileComBlocos(void (*blocoCondicao)(void), void (*blocoCorpo)(void)) {
    char* labelInicio = novaLabel();
    char* labelFim = novaLabel();

    gerarLabel(labelInicio);
    if (blocoCondicao) blocoCondicao();
    gerarGotoFalse(labelFim);

    if (blocoCorpo) blocoCorpo();

    gerarGoto(labelInicio);
    gerarLabel(labelFim);

    free(labelInicio);
    free(labelFim);
}

// Gera estrutura de repetição for (init; cond; iter; corpo)
void gerarForComBlocos(void (*blocoInit)(void), void (*blocoCond)(void), void (*blocoIter)(void), void (*blocoCorpo)(void)) {
    char* labelInicio = novaLabel();
    char* labelFim = novaLabel();

    if (blocoInit) blocoInit();

    gerarLabel(labelInicio);

    if (blocoCond) blocoCond();

    gerarGotoFalse(labelFim);

    if (blocoCorpo) blocoCorpo();

    if (blocoIter) blocoIter();

    gerarGoto(labelInicio);
    gerarLabel(labelFim);

    free(labelInicio);
    free(labelFim);
}

// ============================
// CHAMADAS DE FUNÇÃO
// ============================

// Gera chamada de função com parâmetros e atribuição do retorno
void gerarChamadaFuncao(const char* destino, const char* nomeFunc, char* args[], int qtdArgs) {
    for (int i = 0; i < qtdArgs; i++) {
        gerarComando("PARAM %s", args[i]);
    }
    gerarComando("%s = CALL %s", destino, nomeFunc);
}

// ============================
// SUPORTE A TEMPORÁRIOS
// ============================

// Armazena o último temporário gerado (útil para expressões compostas)
void setUltimoResultado(const char* temp) {
    strncpy(ultimoResultado, temp, sizeof(ultimoResultado));
    ultimoResultado[sizeof(ultimoResultado) - 1] = '\0';
}

// Recupera o último temporário salvo
const char* getUltimoResultado(void) {
    return ultimoResultado;
}

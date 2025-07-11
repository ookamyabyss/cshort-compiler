#include <stdio.h>
#include <string.h>
#include "symbols.h"

// Tabela de símbolos visível globalmente (para debug ou inspeção externa)
Simbolo tabelaSimbolos[MAX_SIMBOLOS];
int numSimbolos = 0;

// Escopo atual do compilador (inicia como global)
Escopo escopoAtual = ESC_GLOBAL;

// Tabela interna real (array estático)
static Simbolo tabela[MAX_TABELA];
static int nSimbolos = 0;

// ===================
// Inicialização
// ===================

// Inicializa a tabela de símbolos (zera o contador)
void inicializarTabela() {
    nSimbolos = 0;
}

// ===================
// Inserção e busca
// ===================

// Insere um novo símbolo na tabela de símbolos
int inserirSimbolo(const char* nome, const char* tipo, Classe classe, Escopo escopo, int tamanho) {
    // Verifica se já existe símbolo com mesmo nome e escopo
    for (int i = 0; i < nSimbolos; i++) {
        if (strcmp(tabela[i].nome, nome) == 0 && tabela[i].escopo == escopo) {
            fprintf(stderr, "Erro: símbolo '%s' já declarado neste escopo.\n", nome);
            return 0;  // erro de duplicação
        }
    }

    // Verifica limite
    if (nSimbolos >= MAX_TABELA) {
        fprintf(stderr, "Erro: tabela de símbolos cheia.\n");
        return 0;
    }

    // Preenche o símbolo
    strncpy(tabela[nSimbolos].nome, nome, sizeof(tabela[nSimbolos].nome));
    strncpy(tabela[nSimbolos].tipo, tipo, sizeof(tabela[nSimbolos].tipo));
    tabela[nSimbolos].classe = classe;
    tabela[nSimbolos].escopo = escopo;
    tabela[nSimbolos].tamanho = tamanho;

    // Todo novo símbolo inserido começa como ATIVO
    tabela[nSimbolos].estado = ESTADO_VIVO;

    nSimbolos++;
    return 1;  // sucesso
}

// Busca um símbolo pelo nome e escopo, respeitando zumbificação e sombreamento
Simbolo* buscarSimbolo(const char* nome, Escopo escopo) {
    // --- ALTERADO ---
    // A busca agora ignora zumbis e respeita o sombreamento de escopo.
    // O parâmetro 'escopo' indica de ONDE a busca se origina.
    for (int i = nSimbolos - 1; i >= 0; i--) {
        // Verifica se o nome bate E se o símbolo está ativo
        if (strcmp(tabela[i].nome, nome) == 0 && tabela[i].estado == ESTADO_VIVO) {
            // Se encontrou um símbolo ativo com o nome certo, ele é um candidato.
            // Se a busca partiu de um escopo local, qualquer símbolo encontrado (local ou global) é válido.
            // Se a busca partiu de um escopo global, apenas um símbolo global é válido.
            if (escopo == ESC_LOCAL) {
                return &tabela[i]; // Retorna o primeiro ativo que encontrar (o mais interno)
            } else if (escopo == ESC_GLOBAL && tabela[i].escopo == ESC_GLOBAL) {
                return &tabela[i]; // Encontrou um global, como pedido
            }
        }
    }
    return NULL; 
}

// Zumbifica todos os símbolos locais ativos (limpa o escopo local)
void limparEscopo(Escopo escopo) {

    if (escopo == ESC_LOCAL) {
        for (int i = nSimbolos - 1; i >= 0; i--) {
            // Para quando chegar no escopo global
            if (tabela[i].escopo == ESC_GLOBAL) {
                break; 
            }
            // Zumbifica o símbolo local se ele estiver ativo
            if (tabela[i].escopo == ESC_LOCAL && tabela[i].estado == ESTADO_VIVO) {
                tabela[i].estado = ESTADO_ZUMBI;
            }
        }
    }
}

// Imprime todos os símbolos cadastrados (para debug)
void imprimirTabela() {
    printf("======= TABELA DE SÍMBOLOS =======\n");
    for (int i = 0; i < nSimbolos; i++) {
        const char* classeStr;
        switch (tabela[i].classe) {
            case CLASSE_VAR: classeStr = "var"; break;
            case CLASSE_VETOR: classeStr = "vetor"; break;
            case CLASSE_FUNCAO: classeStr = "funcao"; break;
            case CLASSE_PARAM: classeStr = "param"; break;
            default: classeStr = "???";
        }

        const char* escopoStr = (tabela[i].escopo == ESC_GLOBAL) ? "global" : "local";
        const char* estadoStr = (tabela[i].estado == ESTADO_VIVO) ? "ATIVO" : "ZUMBI"; 

        printf("Nome: %-10s | Tipo: %-6s | Classe: %-6s | Escopo: %-6s | Tamanho: %d | Estado: %s \n",
               tabela[i].nome,
               tabela[i].tipo,
               classeStr,
               escopoStr,
               tabela[i].tamanho,
               estadoStr);
    }
    printf("==================================\n");
}

// ===================
// Funções auxiliares para o parser
// ===================

// Registra uma variável global (vetor ou não)
void registrarVariavelGlobal(const char* tipo, const char* nome, int isVetor, int tamanho) {
    Classe classe = isVetor ? CLASSE_VETOR : CLASSE_VAR;
    if (!inserirSimbolo(nome, tipo, classe, ESC_GLOBAL, isVetor ? tamanho : 1)) {
        fprintf(stderr, "Erro ao registrar variável global: %s\n", nome);
    }
}

// Registra uma função global
void registrarFuncao(const char* tipo, const char* nome) {
    inserirSimbolo(nome, tipo, CLASSE_FUNCAO, ESC_GLOBAL, 0);
}

// Registra um parâmetro de função (vetor, valor ou por referência)
void registrarParametro(const char* tipo, const char* nome, Classe classe) {
    int tamanho = (classe == CLASSE_VETOR) ? 0 : 1;
    if (!inserirSimbolo(nome, tipo, classe, ESC_LOCAL, tamanho)) {
        fprintf(stderr, "Erro ao registrar parâmetro: %s\n", nome);
    } else {
        printf("[TABELA] Registrado parâmetro: %s | Tipo: %s | Classe: %s\n",
               nome, tipo,
               classe == CLASSE_PARAM ? "param" :
               classe == CLASSE_VETOR ? "vetor param" :
               classe == CLASSE_VAR   ? "var" :
               "desconhecida");
    }
}

// Registra uma variável local (vetor ou não)
void registrarVariavelLocal(const char* tipo, const char* nome, int isVetor, int tamanho) {
    Classe classe = isVetor ? CLASSE_VETOR : CLASSE_VAR;
    if (!inserirSimbolo(nome, tipo, classe, ESC_LOCAL, isVetor ? tamanho : 1)) {
        fprintf(stderr, "Erro ao registrar variável local: %s\n", nome);
    } 
}

// Busca o símbolo mais interno (prioriza local, depois global)
Simbolo* buscarSimboloEmEscopos(const char* nome) {
    for (int i = nSimbolos - 1; i >= 0; i--) {
        if (strcmp(tabela[i].nome, nome) == 0 && tabela[i].estado == ESTADO_VIVO) {
            return &tabela[i]; // O primeiro válido encontrado (mais interno)
        }
    }
    return NULL;
}
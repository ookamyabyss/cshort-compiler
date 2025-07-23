#include <stdio.h>
#include <string.h>
#include "symbols.h"

// Escopo atual do compilador (inicia como global)
Escopo escopoAtual = ESC_GLOBAL;

// Tabela interna real (array estático)
static Simbolo tabela[MAX_TABELA];
static int nSimbolos = 0;

// Retorna o ponteiro para a tabela de símbolos interna do compilador
Simbolo* getTabela() {
    return tabela;
}

// Retorna o número atual de símbolos armazenados na tabela
int getNumSimbolos() {
    return nSimbolos;
}

// ===== Interface pública da tabela de símbolos =====

// Inicializa a tabela de símbolos (zera o contador)
void inicializarTabela() {
    nSimbolos = 0;
}

// Insere um novo símbolo na tabela de símbolos do compilador
int inserirSimbolo(const char* nome, const char* tipo, Classe classe, Escopo escopo, int tamanho) {
    // Verifica se já existe símbolo com mesmo nome, escopo e estado ativo (vivo)
    for (int i = 0; i < nSimbolos; i++) {
        if (strcmp(tabela[i].nome, nome) == 0 && 
            tabela[i].escopo == escopo && 
            tabela[i].estado == ESTADO_VIVO) {
            fprintf(stderr, "Erro: símbolo '%s' já declarado neste escopo.\n", nome);
            return 0;  // duplicação detectada, erro
        }
    }

    // Verifica se a tabela já atingiu o tamanho máximo permitido
    if (nSimbolos >= MAX_TABELA) {
        fprintf(stderr, "Erro: tabela de símbolos cheia.\n");
        return 0;   // tabela cheia, não pode inserir mais
    }

    // Copia os dados do símbolo para a próxima posição da tabela
    strncpy(tabela[nSimbolos].nome, nome, sizeof(tabela[nSimbolos].nome));
    strncpy(tabela[nSimbolos].tipo, tipo, sizeof(tabela[nSimbolos].tipo));
    tabela[nSimbolos].classe = classe;
    tabela[nSimbolos].escopo = escopo;
    tabela[nSimbolos].tamanho = tamanho;

    // Inicializa o estado do símbolo como ativo (vivo)
    tabela[nSimbolos].estado = ESTADO_VIVO;

    // Marca que o símbolo ainda não foi definido (importante para funções)
    tabela[nSimbolos].foiDefinida = false;

    // Incrementa o contador de símbolos registrados
    nSimbolos++;

    // Indica sucesso na inserção
    return 1;  
}

// Busca um símbolo na tabela pelo nome e escopo, respeitando zumbificação (símbolos mortos) e sombreamento de escopo
Simbolo* buscarSimbolo(const char* nome, Escopo escopo) {
    // Percorre a tabela de símbolos de trás para frente, para garantir que o símbolo mais interno (mais recente)
    // seja encontrado primeiro, respeitando o sombreamento de variáveis
    for (int i = nSimbolos - 1; i >= 0; i--) {
        // Verifica se o nome do símbolo bate com o procurado e se o símbolo está ativo (não é zumbi)
        if (strcmp(tabela[i].nome, nome) == 0 && tabela[i].estado == ESTADO_VIVO) {
            // Caso a busca tenha começado num escopo local, qualquer símbolo ativo com o nome é válido (local ou global)
            if (escopo == ESC_LOCAL) {
                return &tabela[i]; // Retorna o símbolo ativo encontrado (mais interno)
            } 
            // Caso a busca tenha começado no escopo global, só considera símbolos globais
            else if (escopo == ESC_GLOBAL && tabela[i].escopo == ESC_GLOBAL) {
                return &tabela[i]; // Retorna símbolo global encontrado
            }
        }
    }
    // Se não encontrou nenhum símbolo ativo que atenda aos critérios, retorna NULL
    return NULL; 
}

// Zumbifica (marca como inativo) todos os símbolos locais ativos, efetivamente limpando o escopo local
void limparEscopo(Escopo escopo) {
    // Só realiza a ação se o escopo passado for o local
    if (escopo == ESC_LOCAL) {
        // Percorre a tabela de símbolos de trás para frente (últimos inseridos primeiro)
        for (int i = nSimbolos - 1; i >= 0; i--) {
            // Verifica se o símbolo é local e está ativo
            if (tabela[i].escopo == ESC_LOCAL && tabela[i].estado == ESTADO_VIVO) {
                // Marca o símbolo como zumbi (inativo), para que não seja mais considerado nas buscas
                tabela[i].estado = ESTADO_ZUMBI;
            }
        }
    }
}

// Imprime todos os símbolos cadastrados na tabela de símbolos para fins de depuração
void imprimirTabela() {
    // Cabeçalho da tabela
    printf("======= TABELA DE SÍMBOLOS =======\n");

    // Percorre todos os símbolos cadastrados
    for (int i = 0; i < nSimbolos; i++) {
        // Converte a enumeração da classe do símbolo para uma string legível
        const char* classeStr;
        switch (tabela[i].classe) {
            case CLASSE_VAR:    classeStr = "var";     break;  // variável simples
            case CLASSE_VETOR:  classeStr = "vetor";   break;  // vetor/array
            case CLASSE_FUNCAO: classeStr = "funcao";  break;  // função
            case CLASSE_PARAM:  classeStr = "param";   break;  // parâmetro de função
            default:            classeStr = "???";                 // classe desconhecida
        }

        // Converte o escopo do símbolo em string: global ou local
        const char* escopoStr = (tabela[i].escopo == ESC_GLOBAL) ? "global" : "local";

        // Converte o estado do símbolo em string: ativo (vivo) ou zumbi (inativo)
        const char* estadoStr = (tabela[i].estado == ESTADO_VIVO) ? "ATIVO" : "ZUMBI";

        // Imprime uma linha com as informações formatadas do símbolo
        printf("Nome: %-10s | Tipo: %-6s | Classe: %-6s | Escopo: %-6s | Tamanho: %d | Estado: %s \n",
               tabela[i].nome,       // nome do símbolo
               tabela[i].tipo,       // tipo do símbolo (ex: int, float)
               classeStr,            // classe do símbolo (variável, função, etc)
               escopoStr,            // escopo do símbolo (global ou local)
               tabela[i].tamanho,    // tamanho (ex: tamanho de vetor)
               estadoStr);           // estado atual (ativo ou zumbi)
    }

    // Rodapé da tabela
    printf("==================================\n");
}

// ===== Funções auxiliares chamadas pelo parser =====

// Registra uma variável global na tabela de símbolos
void registrarVariavelGlobal(const char* tipo, const char* nome, int isVetor, int tamanho) {
    // Determina a classe do símbolo: vetor se isVetor == 1, senão variável simples
    Classe classe = isVetor ? CLASSE_VETOR : CLASSE_VAR;

    if (!inserirSimbolo(nome, tipo, classe, ESC_GLOBAL, isVetor ? tamanho : 1)) {
        // Se falhar a inserção (ex: duplicação ou tabela cheia), exibe mensagem de erro
        fprintf(stderr, "Erro ao registrar variável global: %s\n", nome);
    }
}

// Registra uma função global na tabela de símbolos, seja um protótipo (declaração) ou definição
void registrarFuncao(const char* tipo, const char* nome, int nParams, char tiposParams[][10]) {
    // Busca se já existe um símbolo com esse nome no escopo global
    Simbolo* existente = buscarSimbolo(nome, ESC_GLOBAL);

    // Se já existe uma função com esse nome que ainda não foi definida (protótipo)
    if (existente && existente->classe == CLASSE_FUNCAO && !existente->foiDefinida) {
        // Atualiza o tipo de retorno da função
        strncpy(existente->tipo, tipo, sizeof(existente->tipo));
        // Atualiza o número de parâmetros
        existente->nParams = nParams;

        // Atualiza os tipos dos parâmetros da função
        for (int i = 0; i < nParams; i++) {
            strncpy(existente->tiposParams[i], tiposParams[i], sizeof(existente->tiposParams[i]));
        }

        // Retorna, já atualizou o protótipo existente
        return;
    }

    // Se a função não existe ou já foi definida anteriormente, tenta inserir uma nova
    int ok = inserirSimbolo(nome, tipo, CLASSE_FUNCAO, ESC_GLOBAL, 0);
    if (!ok) return;    // se não conseguiu inserir, aborta

    // Pega um ponteiro para o símbolo recém-inserido (último da tabela)
    Simbolo* func = &tabela[nSimbolos - 1]; 

    // Define o número de parâmetros da função
    func->nParams = nParams;

    // Copia os tipos dos parâmetros para o símbolo da função
    for (int i = 0; i < nParams; i++) {
        strncpy(func->tiposParams[i], tiposParams[i], sizeof(func->tiposParams[i]));
    }
}

// Registra um parâmetro de função na tabela de símbolos.
void registrarParametro(const char* tipo, const char* nome, Classe classe, Escopo escopo, int tamanho) {
    // Tenta inserir o símbolo do parâmetro na tabela de símbolos.
    // Caso falhe (ex: nome duplicado no mesmo escopo), imprime erro.
    if (!inserirSimbolo(nome, tipo, classe, escopo, tamanho)) {
        fprintf(stderr, "Erro ao registrar parâmetro: %s\n", nome);
    }
}

// Registra uma variável local na tabela de símbolos do compilador
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

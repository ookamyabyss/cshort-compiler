#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "semantic.h"
#include "symbols.h"
#include "lexer.h" 
#include "parser.h"

static const char* nomeFuncaoAtual = NULL;

static bool encontrouReturnComValor = false;

// Armazena o tipo da variável no lado esquerdo da atribuição 
static const char* tipoAtribuido = NULL;

// Armazena o tipo da expressão do lado direito da atribuição 
static const char* tipoExpressao = NULL;

// Escopo atual de análise (global ou local)
extern Escopo escopoAtual;

// ==============================================
// INTERFACE DO ANALISADOR SEMÂNTICO - C.SHORT
// ==============================================

// ----------------------------------------------
// Mensagens de erro e finalização
// ----------------------------------------------

// Emite uma mensagem de erro semântico e encerra o compilador
void erroSemantico(const char* msg, const char* nome) {
    fprintf(stderr, "[ERRO SEMÂNTICO] %s: %s\n", nome, msg);
    exit(1);
}

// Finaliza a análise semântica com mensagem de sucesso (placeholder)
void verificarSemantica() {
    printf("[OK] Análise semântica concluída com sucesso.\n");
}

// ----------------------------------------------
// 1. Declaração e uso de variáveis
// ----------------------------------------------

// Verifica se uma variável (ou vetor) foi previamente declarada
void verificarVariavelDeclarada(const char* nome) {
    Simbolo* s = buscarSimboloEmEscopos(nome); // <- agora passando escopo
    if (s == NULL) {
        erroSemantico("Variável não declarada", nome);
    }
}

// Verifica se identificador já foi declarado no mesmo escopo
void verificarRedeclaracao(const char* nome) {
    Simbolo* existente = buscarSimbolo(nome, escopoAtual);

    if (existente != NULL) {
        // Se for função:
        if (existente->classe == CLASSE_FUNCAO) {
            // Permite se ainda não foi definida (ou seja, é um protótipo)
            if (!existente->foiDefinida) {
                return;  // ok, vai ser marcada como definida depois
            }
        }

        // Caso contrário, é erro
        erroSemantico("Identificador já declarado no mesmo escopo", nome);
    }
}

// Inicia verificação de atribuição (armazenando o tipo da variável à esquerda)
void iniciarAtribuicao(const char* nome) {
    Simbolo* s = buscarSimboloEmEscopos(nome);
    if (s == NULL) {
        erroSemantico("Identificador não declarado antes da atribuição", nome);
    }

    if (s->classe == CLASSE_FUNCAO) {
        erroSemantico("Função usada como variável na atribuição", nome);
    }

    garantirTipoDefinido(s->tipo, s->nome);

    tipoAtribuido = s->tipo;
    tipoExpressao = NULL; 
}

// Registra o tipo da expressão analisada (lado direito da atribuição)
void registrarTipoExpressao(const char* tipo) {
    tipoExpressao = tipo;
}

// Verifica se tipos na atribuição (esquerda e direita) são compatíveis
void verificarTipoExpr() {
    if (tipoAtribuido == NULL || tipoExpressao == NULL) return;

    if (!tiposSaoCompatíveis(tipoAtribuido, tipoExpressao)) {
        char msg[128];
        snprintf(msg, sizeof(msg),
            "Tipo incompatível na atribuição: esperado '%s', mas recebeu '%s'",
            tipoAtribuido, tipoExpressao);
        erroSemantico(msg, "");
    }
}

// Registra tipo de constante literal (int, float, char, bool)
void registrarTipoConstante(Token token) {
    switch (token.type) {
        case TOKEN_INTCON:
            registrarTipoExpressao("int");
            break;
        case TOKEN_REALCON:
            registrarTipoExpressao("float");
            break;
        case TOKEN_CHARCON:
        case TOKEN_CHARCON_0:
        case TOKEN_CHARCON_N:
            registrarTipoExpressao("char");
            break;
        case TOKEN_BOOLCON:
            registrarTipoExpressao("bool");
            break;
        case TOKEN_STRINGCON:
            registrarTipoExpressao("char[]");
            break;
        default:
            break;
    }
}

// Analisa token atual e registra tipo, se aplicável (constantes ou identificadores)
void analisarTokenAtual(Token token) {
    // Constante literal? Registra o tipo normalmente
    registrarTipoConstante(token);

    // Identificador? Pode ser variável OU função chamada numa expressão
    if (token.type == TOKEN_ID) {
        Simbolo* s = buscarSimboloEmEscopos(token.lexeme);

        if (s == NULL) {
            erroSemantico("Identificador usado mas não declarado", token.lexeme);
        }

        garantirTipoDefinido(s->tipo, s->nome);

        // Se for vetor (tipo termina com "[]"), registrar tipo base
        if (strstr(s->tipo, "[]") != NULL) {
            char tipoBase[10];
            strncpy(tipoBase, s->tipo, strlen(s->tipo) - 2);
            tipoBase[strlen(s->tipo) - 2] = '\0';
            registrarTipoExpressao(tipoBase);
        } else {
            registrarTipoExpressao(s->tipo);
        }
    }
}

// ----------------------------------------------
// 2. Funções - declarações e uso
// ----------------------------------------------

// Verifica se identificador chamado é uma função válida
void registrarChamadaDeFuncao(const char* nome) {
    Simbolo* s = buscarSimboloEmEscopos(nome);
    if (s == NULL) {
        erroSemantico("Função chamada mas não declarada", nome);
    }
    if (s->classe != CLASSE_FUNCAO) {
        erroSemantico("Identificador chamado como função, mas não é uma função", nome);
    }

    garantirTipoDefinido(s->tipo, s->nome);

    registrarTipoExpressao(s->tipo); // permite verificar o tipo de retorno em atribuições
}

// Verifica se definição de função está correta e marca como "definida"
void verificarDefinicaoDeFuncao(const char* nome) {
    Simbolo* s = buscarSimbolo(nome, ESC_GLOBAL);

    if (s != NULL) {
        if (s->classe != CLASSE_FUNCAO) {
            erroSemantico("Identificador já declarado como não função", nome);
        }
        if (s->foiDefinida) {
            erroSemantico("Função já foi definida anteriormente", nome);
        }

        // Protótipo já existia, marca como definida agora
        s->foiDefinida = true;
        return;
    }

    // Se não existia antes, é uma definição nova
    int ok = inserirSimbolo(nome, "tipo", CLASSE_FUNCAO, ESC_GLOBAL, 0);
    if (!ok) {
        erroSemantico("Erro ao definir função", nome);
    }

    // Marcar como definida o último símbolo real da tabela
    Simbolo* tabela = getTabela();
    int n = getNumSimbolos();
    tabela[n - 1].foiDefinida = true;
}

// Verifica se assinatura da definição bate com o protótipo anterior
void verificarAssinaturaCompatível(const char* nome, const char* tipoRetorno, int nParams, char tiposParams[][10]) {
    Simbolo* s = buscarSimbolo(nome, ESC_GLOBAL);
    if (!s || s->classe != CLASSE_FUNCAO) return;
   
    if (strcmp(s->tipo, tipoRetorno) != 0) {
        erroSemantico("Tipo de retorno da definição não bate com o protótipo", nome);
    }

    if (s->nParams != nParams) {
        erroSemantico("Número de parâmetros da definição não bate com o protótipo", nome);
    }

    for (int i = 0; i < nParams; i++) {
        if (strcmp(s->tiposParams[i], tiposParams[i]) != 0) {
            erroSemantico("Tipo de parâmetro incompatível com o protótipo", nome);
        }
    }
}

// Verifica se há parâmetro repetido na lista de parâmetros formais
void verificarParametroRepetido(const char* nome) {
    for (int i = 0; i < numParamsTemp; i++) {
        if (strcmp(nomesParamsTemp[i], nome) == 0) {
            erroSemantico("Parâmetro repetido na lista de parâmetros formais", nome);
        }
    }
}

// Verifica se função sem parâmetros declarou `void` explicitamente
void verificarVoidEmFuncaoSemParametros(int nParams, char tiposParams[][10], const char* nome) {
    if (nParams == 0) {
        erroSemantico("Função sem parâmetros deve declarar void explicitamente", nome);
    }

    if (nParams == 1 && strcmp(tiposParams[0], "void") == 0) {
        return; // ok
    }
}

// Verifica se o tipo de uma variável ou função está corretamente definido
void garantirTipoDefinido(const char* tipo, const char* nome) {
    if (tipo == NULL || strcmp(tipo, "") == 0 || strcmp(tipo, "tipo") == 0) {
        erroSemantico("Tipo da variável ou função não foi definido corretamente", nome);
    }
}

// Retorna se dois tipos são semanticamente compatíveis
bool tiposSaoCompatíveis(const char* tipo1, const char* tipo2) {
    // Se algum for "erro", nunca é compatível
    if (strcmp(tipo1, "erro") == 0 || strcmp(tipo2, "erro") == 0) return false;

    // Mesmos tipos
    if (strcmp(tipo1, tipo2) == 0) return true;

    // int <-> char
    if ((strcmp(tipo1, "int") == 0 && strcmp(tipo2, "char") == 0) ||
        (strcmp(tipo1, "char") == 0 && strcmp(tipo2, "int") == 0)) {
        return true;
    }

    // bool <-> int (permitido em atribuição)
    if ((strcmp(tipo1, "bool") == 0 && strcmp(tipo2, "int") == 0) ||
        (strcmp(tipo1, "int") == 0 && strcmp(tipo2, "bool") == 0)) {
        return true;
    }

    // Vetores idênticos
    if ((strcmp(tipo1, "int[]") == 0 && strcmp(tipo2, "int[]") == 0) ||
        (strcmp(tipo1, "char[]") == 0 && strcmp(tipo2, "char[]") == 0)) {
        return true;
    }

    // Incompatível
    return false;
}

// Verifica se função com retorno está sendo usada como expressão
void verificarUsoDeFuncaoEmExpressao(const char* nome) {
    Simbolo* s = buscarSimboloEmEscopos(nome);
    if (!s || s->classe != CLASSE_FUNCAO) {
        erroSemantico("Identificador chamado como função, mas não é uma função", nome);
    }

    garantirTipoDefinido(s->tipo, s->nome);

    if (strcmp(s->tipo, "void") == 0) {
        erroSemantico("Função 'void' não pode ser usada como expressão", nome);
    }

    registrarTipoExpressao(s->tipo);
}

// Verifica se função com valor de retorno está sendo usada como comando
void verificarUsoDeFuncaoComoComando(const char* nome) {
    Simbolo* s = buscarSimboloEmEscopos(nome);
    if (!s || s->classe != CLASSE_FUNCAO) {
        erroSemantico("Identificador chamado como função, mas não é uma função", nome);
    }

    garantirTipoDefinido(s->tipo, s->nome);

    if (strcmp(s->tipo, "void") != 0) {
        erroSemantico("Função com valor de retorno usada como comando", nome);
    }
}

// Verifica se há erro de retorno de valor em função `void`
void verificarReturnComValor() {
    if (!nomeFuncaoAtual) return;

    Simbolo* func = buscarSimbolo(nomeFuncaoAtual, ESC_GLOBAL);
    if (!func || func->classe != CLASSE_FUNCAO) return;

    if (strcmp(func->tipo, "void") == 0) {
        erroSemantico("Função 'void' não pode retornar valor", func->nome);
    }

    encontrouReturnComValor = true;  // <-- marca que houve retorno com valor

}

// Verifica se há erro de `return;` em função com retorno
void verificarReturnSemValor() {
    if (!nomeFuncaoAtual) return;

    Simbolo* func = buscarSimbolo(nomeFuncaoAtual, ESC_GLOBAL);
    if (!func || func->classe != CLASSE_FUNCAO) return;

    if (strcmp(func->tipo, "void") != 0) {
        erroSemantico("Função com valor de retorno exige 'return' com valor", func->nome);
    }
}

// Armazena o nome da função atualmente sendo analisada
void setFuncaoAtual(const char* nome) {
    nomeFuncaoAtual = nome;
    encontrouReturnComValor = false;  // reset ao entrar na função

}

// Verifica se função com tipo de retorno tem pelo menos um `return expr;`
void verificarFuncaoComRetornoObrigatorio() {
    if (!nomeFuncaoAtual) return;

    Simbolo* func = buscarSimbolo(nomeFuncaoAtual, ESC_GLOBAL);
    if (!func || func->classe != CLASSE_FUNCAO) return;

    if (strcmp(func->tipo, "void") != 0 && !encontrouReturnComValor) {
        erroSemantico("Função com valor de retorno deve conter pelo menos um 'return expr;'", func->nome);
    }
}

void registrarTipoRelacional() {
    registrarTipoExpressao("bool");
}

void registrarTipoLogico() {
    registrarTipoExpressao("bool");
}

const char* tipoDominanteAritmetico(const char* t1, const char* t2) {
    // Se algum dos dois for vetor, não é permitido
    if (tipoEhVetor(t1) || tipoEhVetor(t2)) {
        fprintf(stderr, "[ERRO SEMÂNTICO] Operações aritméticas não são permitidas com vetores\n");
        return "erro";
    }

    // Só aceitamos int e char como tipos válidos
    bool valido1 = strcmp(t1, "int") == 0 || strcmp(t1, "char") == 0;
    bool valido2 = strcmp(t2, "int") == 0 || strcmp(t2, "char") == 0;

    if (!valido1 || !valido2) {
        fprintf(stderr, "[ERRO SEMÂNTICO] Tipos incompatíveis para operação aritmética: %s e %s\n", t1, t2);
        return "erro";
    }

    // Regra: int + char → int
    if (strcmp(t1, "int") == 0 || strcmp(t2, "int") == 0) {
        return "int";
    }

    // Se ambos forem char, resultado é char
    return "char";
}

const char* getTipoExpressao() {
    return tipoExpressao;
}

void setTipoExpressao(const char* tipo) {
    tipoExpressao = tipo;
}

bool tipoEhVetor(const char* tipo) {
    return strstr(tipo, "[]") != NULL;
}

static char ultimoTipoExpr[16] = "";

void setUltimoTipoExpr(const char* tipo) {
    strncpy(ultimoTipoExpr, tipo, sizeof(ultimoTipoExpr));
}

const char* getUltimoTipoExpr() {
    return ultimoTipoExpr;
}
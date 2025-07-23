#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include "parser.h"
#include "lexer.h"
#include "symbols.h"
#include "semantic.h"
#include "codegen.h"

// ==============================
// Variáveis globais
// ==============================

// Armazena os tipos dos parâmetros encontrados
char tiposParamsTemp[MAX_PARAMS_FUNCAO][10];
char nomesParamsTemp[MAX_PARAMS_FUNCAO][256];
int numParamsTemp = 0;

// Token atualmente em análise (lookahead principal usado pelo parser)
static Token currentToken;     

// Token salvo temporariamente para permitir "voltar" um passo (backtracking simples)
static Token backupToken;      

// Token empurrado manualmente por alguma função (por exemplo, ungetToken)
static Token pushedToken;      

// Flag indicando se existe um token empurrado e aguardando ser usado
static int hasPushedToken = 0;  

// Flag que permite um "retrocesso" simples: reprocessar o último token
static bool tokenBack = false; 

// ==============================
// Controle de Tokens
// ==============================

// Avança para o próximo token.
void advance() {
    if (tokenBack) {
        tokenBack = false;
        // Usa o backupToken em vez de pegar novo token
        currentToken = backupToken;
    } else {
        currentToken = getNextToken();
    }
}

// Salva um token para ser lido novamente no próximo `advance()`.
void pushBackToken(Token t) {
    pushedToken = t;
    hasPushedToken = 1;
}

// Marca que o token atual deve ser reutilizado.
void ungetToken(Token t) {
    backupToken = t;
    tokenBack = true;
}

// ==============================
// Erros
// ==============================

// Função para reportar erros sintáticos e encerrar.
static void parseError(const char* message) {
    fprintf(stderr, "[ERRO SINTÁTICO] %s. Encontrado '%s' (tipo %d) na linha %d, coluna %d.\n",
            message, currentToken.lexeme, currentToken.type, currentToken.line, currentToken.column);
    exit(EXIT_FAILURE);
}

// Verifica se o token atual é do tipo esperado e o consome (avança)
void parseEat(int expectedType) {
    if (currentToken.type == expectedType) {
        advance();  // Consome o token atual e avança para o próximo
    } else {
        // Erro: token inesperado
        fprintf(stderr, "[ERRO SINTÁTICO] Esperado token do tipo %d, mas encontrado '%s' (linha %d, coluna %d)\n",
                expectedType, currentToken.lexeme, currentToken.line, currentToken.column);
        exit(EXIT_FAILURE); // Encerra o programa devido ao erro
    }
}

// ==============================
// Entrada do Parser
// ==============================

// Ponto de entrada do analisador sintático (parser).
void startParser(FILE* f) {
    initLexer(f);       // Inicializa o analisador léxico com o arquivo de entrada
    advance();          // Carrega o primeiro token (lookahead)
    parseProg();        // Inicia a análise sintática a partir do símbolo inicial da gramática
    printf("[OK] Análise sintática concluída com sucesso.\n");
    destroyLexer();     // Libera recursos usados pelo analisador léxico
}

// prog ::= { decl ';' | func } 
// Função principal de análise sintática do programa.
// Reconhece e processa uma sequência de declarações de variáveis e funções.
void parseProg() {
    
    // Enquanto não chegar ao fim do arquivo fonte
    while (currentToken.type != TOKEN_EOF) {

        // Verifica se o token atual é um tipo válido (int, float, char, bool)
        if (isTipo(currentToken.type)) {
            // Pode ser tanto uma declaração de variável quanto de função
            // Chama parseDecl para decidir com base nos próximos tokens
            parseDecl();

        // Caso o token seja 'void', só pode ser uma função (sem valor de retorno)
        } else if (currentToken.type == TOKEN_KEYWORD_VOID) {
            // Chama parseDecl para tratar a declaração da função void
            parseDecl();

        // Se não for um tipo válido nem 'void', gera um erro sintático
        } else {
            parseError("Esperado tipo ou void");
        }
    }
}

// Analisador de declarações (variáveis ou funções)
// decl ::= tipo decl_var {...} | tipo id(...) {...} | void id(...) {...}
void parseDecl() {

    // Caso comece com um tipo (int, float, char, bool)
    if (isTipo(currentToken.type)) {

        char tipoStr[10];
        obterTipoString(tipoStr);  // Captura o tipo como string ("int", "char", etc.)
        parseTipo();               // Avança o token do tipo

        // Espera um identificador depois do tipo
        if (currentToken.type == TOKEN_ID) {
            char nomeFunc[256];
            strncpy(nomeFunc, currentToken.lexeme, sizeof(nomeFunc)); // Guarda o nome do identificador
            nomeFunc[sizeof(nomeFunc) - 1] = '\0';
            advance(); // Consome o identificador

            // Se o próximo token for '(', é uma função
            if (currentToken.type == TOKEN_LPAREN) {
     
                advance();                    // consome '('
                parseTiposParam();            // Processa os parâmetros da função

                // Verificações semânticas
                verificarAssinaturaCompatível(nomeFunc, tipoStr, numParamsTemp, tiposParamsTemp);
                verificarRedeclaracao(nomeFunc); // Verifica duplicidade
                registrarFuncao(tipoStr, nomeFunc, numParamsTemp, tiposParamsTemp); // Registra na tabela
                printf("[DECL_FUNCAO] Função com tipo reconhecida: %s\n", nomeFunc);

                parseEat(TOKEN_RPAREN); // Fecha os parênteses dos parâmetros

                // Suporte para declarações múltiplas de funções separadas por vírgula
                while (currentToken.type == TOKEN_COMMA) {
                    advance();                      // Consome ','
                    parseEat(TOKEN_ID);             // Consome novo nome de função
                    printf("[DECL_FUNCAO] Função adicional reconhecida: %s\n", currentToken.lexeme);
                    parseEat(TOKEN_LPAREN);         // Abre parênteses
                    parseTiposParam();              // Novos parâmetros
                    parseEat(TOKEN_RPAREN);         // Fecha parênteses
                }

            // Pode ser protótipo ou definição
            if (currentToken.type == TOKEN_SEMICOLON) {
                verificarVoidEmFuncaoSemParametros(numParamsTemp, tiposParamsTemp, nomeFunc);
                verificarRedeclaracao(nomeFunc);    // Reforça checagem
                advance();                          // Consome ';'
                limparEscopo(ESC_LOCAL);            // Limpa escopo local do protótipo
            } else if (currentToken.type == TOKEN_LBRACE) {
                // Definição da função
                verificarAssinaturaCompatível(nomeFunc, tipoStr, numParamsTemp, tiposParamsTemp);
                verificarDefinicaoDeFuncao(nomeFunc); // Garante que não foi redefinida
                setFuncaoAtual(nomeFunc); // Guarda nome da função para escopo
                escopoAtual = ESC_LOCAL;  // Define escopo local
                parseFunc();              // Analisa corpo da função
                escopoAtual = ESC_GLOBAL; // Retorna para escopo global
            } else {
                parseError("Esperado ';' ou '{' após declaração de função");
            }
            } else {
                // Não é função → é declaração de variável
                printf("[DECL] Reconhecida declaração de variável (primeiro ID: %s)\n", nomeFunc);

                int isVetor = 0;
                int tamanho = 1;

                // Verifica se é vetor (ex: int v[10];)
                if (currentToken.type == TOKEN_LBRACK) {
                    advance();
                    if (currentToken.type == TOKEN_INTCON) {
                        tamanho = atoi(currentToken.lexeme);
                        isVetor = 1;
                        printf("[DECL_VAR] Vetor de tamanho: %s\n", currentToken.lexeme);
                        advance();
                        parseEat(TOKEN_RBRACK);
                    } else {
                        parseError("Esperado número inteiro dentro dos colchetes após o identificador");
                    }
                }

                // Verifica e registra variável global
                verificarRedeclaracao(nomeFunc);
                registrarVariavelGlobal(tipoStr, nomeFunc, isVetor, tamanho);

                // Suporte a múltiplas variáveis: ex: int a, b[5];
                if (currentToken.type == TOKEN_LBRACK) {
                    advance(); // consome '['

                    if (currentToken.type == TOKEN_INTCON) {
                        printf("[DECL_VAR] Vetor de tamanho: %s\n", currentToken.lexeme);
                        advance(); // consome número
                        parseEat(TOKEN_RBRACK); // consome ']'
                    } else {
                        parseError("Esperado número inteiro dentro dos colchetes após o identificador");
                    }
                }

                // Agora trata as outras variáveis separadas por vírgula
                while (currentToken.type == TOKEN_COMMA) {
                    advance(); // consome ','
                    parseDeclVar(tipoStr, ESC_GLOBAL); // processa próxima variável
                }

                parseEat(TOKEN_SEMICOLON); // Fecha a declaração
            }
        } else {
            parseError("Esperado identificador após tipo");
        }

    // Caso seja função com tipo void
    } else if (currentToken.type == TOKEN_KEYWORD_VOID) {

        parseEat(TOKEN_KEYWORD_VOID);

        char nomeFunc[256];
        if (currentToken.type == TOKEN_ID) {
            strncpy(nomeFunc, currentToken.lexeme, sizeof(nomeFunc));
            nomeFunc[sizeof(nomeFunc) - 1] = '\0';
        }

        parseEat(TOKEN_ID); // Nome da função
        printf("[DECL_FUNCAO_VOID] Função void reconhecida: %s\n", nomeFunc);
        
        verificarRedeclaracao(nomeFunc); // Verificação semântica
        registrarFuncao("void", nomeFunc, numParamsTemp, tiposParamsTemp); // Registra função void

        parseEat(TOKEN_LPAREN);
        parseTiposParam(); // Parâmetros
        parseEat(TOKEN_RPAREN);

        // Suporte a múltiplas declarações void
        while (currentToken.type == TOKEN_COMMA) {
            advance();
            parseEat(TOKEN_ID);
            printf("[DECL_FUNCAO_VOID] Função void adicional: %s\n", currentToken.lexeme);
            parseEat(TOKEN_LPAREN);
            parseTiposParam();
            parseEat(TOKEN_RPAREN);
        }

        // Pode ser apenas um protótipo ou a definição
        if (currentToken.type == TOKEN_SEMICOLON) {
            advance(); // Consome ';'
        } else if (currentToken.type == TOKEN_LBRACE) {
            verificarAssinaturaCompatível(nomeFunc, "void", numParamsTemp, tiposParamsTemp);
            verificarDefinicaoDeFuncao(nomeFunc);
            setFuncaoAtual(nomeFunc);
            limparEscopo(ESC_LOCAL);
            escopoAtual = ESC_LOCAL;
            parseFunc();    // Analisa corpo da função void
            escopoAtual = ESC_GLOBAL;
        } else {
            parseError("Esperado ';' ou '{' após declaração de função void");
        }

    } else {
        parseError("Esperado tipo ou void na declaração");
    }
}

// Analisa uma declaração de variável (com ou sem colchetes para vetor).
// decl_var ::= id [ '[' intcon ']' ]
void parseDeclVar(const char* tipo, Escopo escopo) {
    char nomeVar[256];   // Buffer para armazenar o nome da variável
    int isVetor = 0;     // Flag para indicar se é vetor (1) ou não (0)
    int tamanho = 1;     // Tamanho do vetor (1 por padrão, se for variável escalar)

    // Copia o lexema atual (nome do identificador) para nomeVar
    strncpy(nomeVar, currentToken.lexeme, sizeof(nomeVar));
    nomeVar[sizeof(nomeVar) - 1] = '\0';    // Garante terminação da string

    // Espera e consome um identificador (nome da variável)
    parseEat(TOKEN_ID);
    printf("[DECL_VAR] Reconhecida variável: %s\n", nomeVar);

    // Verifica se é uma declaração de vetor (se houver colchete abrindo)
    if (currentToken.type == TOKEN_LBRACK) {
        isVetor = 1;  // Marca como vetor
        advance();    // Consome o '['

        // Espera um número inteiro como tamanho do vetor
        if (currentToken.type == TOKEN_INTCON) {
            tamanho = atoi(currentToken.lexeme);       // Converte string para inteiro
            printf("[DECL_VAR] Vetor de tamanho: %d\n", tamanho);
            advance();      // Consome o número
            parseEat(TOKEN_RBRACK); // Espera e consome o ']'
        } else {
            parseError("Esperado número inteiro dentro dos colchetes");
        }
    }

    // Verificação semântica: impede redeclaração do mesmo nome
    verificarRedeclaracao(nomeVar);

    // Registra a variável na tabela de símbolos como global (por enquanto fixo)
    registrarVariavelGlobal(tipo, nomeVar, isVetor, tamanho);
}

// Reconhece um tipo primitivo da linguagem: char, int, float ou bool
// tipo ::= char | int | float | bool 
void parseTipo() {
    // Verifica se o token atual é um dos tipos esperados
    if (isTipo(currentToken.type)) {
        // Se for um tipo válido, avança para o próximo token
        advance();
    } else {
        // Se não for, emite erro sintático
        parseError("Esperado tipo (int, float, char, bool)");
    }
}

// Analisa e armazena os tipos e nomes dos parâmetros formais de uma função
// tipos_param ::= void | tipo (id | &id | id[]){, tipo (...)}
void parseTiposParam() {
    // Inicializa o número temporário de parâmetros
    numParamsTemp = 0;

    // Limpa os arrays temporários que armazenam os nomes e tipos dos parâmetros
    for (int i = 0; i < MAX_PARAMS_FUNCAO; i++) {
        nomesParamsTemp[i][0] = '\0';      // zera a string (nome)
        tiposParamsTemp[i][0] = '\0';      // zera a string (tipo)
    }

    // Caso especial: função sem parâmetros (apenas ')')
    if (currentToken.type == TOKEN_RPAREN) {
        return;  // não há parâmetros
    }

    // Caso especial: função declarada como void
    if (currentToken.type == TOKEN_KEYWORD_VOID) {
        // Armazena "void" como tipo único do parâmetro (indicando ausência real de parâmetros)
        strcpy(tiposParamsTemp[0], "void");
        numParamsTemp = 1;

        // Avança para o próximo token após "void"
        advance();

        // Se houver mais tokens após "void", é um erro (ex: void, int x)
        if (currentToken.type != TOKEN_RPAREN) {
            parseError("Token 'void' não pode ser seguido por outros parâmetros");
        }

        return;  // encerra o processamento dos parâmetros
    }

    // Caso comum: ao menos um parâmetro com tipo e nome
    parseTipoParam();  // analisa o primeiro parâmetro (tipo + nome)

    // Enquanto houver vírgula, continua analisando os próximos parâmetros
    while (currentToken.type == TOKEN_COMMA) {
        advance();          // consome a vírgula
        parseTipoParam();  // analisa o próximo parâmetro
    }

}

// Analisa e traduz a definição de uma função
// func ::= tipo/void id(...) '{' {decl_var} {cmd} '}'
void parseFunc() {

    // Espera e consome o token '{', que abre o corpo da função
    parseEat(TOKEN_LBRACE);

    // Enquanto houver declarações de variáveis locais (começando por um tipo)
    while (isTipo(currentToken.type)) {
        char tipoStr[10];

        // Copia o tipo (ex: "int", "char", etc) para uma string temporária
        obterTipoString(tipoStr);  // ← Isso obtém o tipo em string

        // Consome o token de tipo (ex: "int", "char")
        parseTipo();

        // Lê a primeira variável da linha de declaração
        parseDeclVarPrimeiro(tipoStr, ESC_LOCAL);

        // Lê o restante das variáveis da mesma linha, se houver (ex: int a, b;)
        parseDeclVarResto(tipoStr, ESC_LOCAL);

        // Espera e consome o ponto e vírgula que termina a declaração
        parseEat(TOKEN_SEMICOLON);
    }

    // Após as declarações, processa os comandos da função (corpo)
    while (currentToken.type != TOKEN_RBRACE && currentToken.type != TOKEN_EOF) {
        parseCmd();
    }

    // Verifica se a função possui ao menos um `return` com valor (se necessário)
    verificarFuncaoComRetornoObrigatorio();

    // Espera e consome o token '}' que fecha o corpo da função
    parseEat(TOKEN_RBRACE);

    // Remove as variáveis locais da tabela de símbolos (fim do escopo local)
    limparEscopo(ESC_LOCAL);
}

// Analisa e traduz comandos da linguagem de entrada
// cmd ::= if, while, for, return, atrib, chamada, bloco, ';'
void parseCmd() {
    if (currentToken.type == TOKEN_KEYWORD_IF) {
        // Comando if
        printf("[CMD] Reconhecido comando 'if'\n");
        advance();  // consome 'if'

        parseEat(TOKEN_LPAREN); // espera e consome '('

        // Analisa a expressão simples à esquerda da condição (ex: A)
        char* esq = parseExprSimp();

        // Operador relacional (ex: ">", "<=", "==", "!=")
        char op[4];
        strcpy(op, currentToken.lexeme);
        advance();  // consome operador

        // Analisa a expressão simples à direita da condição (ex: B)
        char* dir = parseExprSimp();

        parseEat(TOKEN_RPAREN);

        // Cria labels para controle de fluxo do if/else
        char* labelFim  = novaLabel();   // Label para fim do if/else
        char* labelThen = novaLabel();   // Label para o bloco then (if)

        // Geração de código para avaliar a condição: carrega os dois valores e subtrai
        gerarLoad(esq);
        gerarLoad(dir);
        gerarSub(); // A - B

        // De acordo com o operador relacional, gera saltos condicionais apropriados
        if (strcmp(op, ">") == 0 || strcmp(op, "!=") == 0) {
            gerarGotoTrue(labelThen);    // Se (A - B) > 0 ou diferente de zero entra no if
            gerarGoto(labelFim);         // Senão pula para o fim do if
        } else if (strcmp(op, "<") == 0 || strcmp(op, "==") == 0) {
            gerarGotoFalse(labelThen);   // Se (A - B) == 0 ou menor que zero entra no if
            gerarGoto(labelFim);
        } else if (strcmp(op, ">=") == 0) {
            gerarGotoFalse(labelFim);    // Se resultado < 0, pula para o fim
            gerarGoto(labelThen);        // Senão entra no if
        } else if (strcmp(op, "<=") == 0) {
            gerarGotoTrue(labelThen);    // Se resultado <= 0, entra no if
            gerarGoto(labelFim);
        }

        // Gera o label para o bloco then e parseia o comando associado
        gerarLabel(labelThen);
        parseCmd();

        // Se houver else, trata a parte else
        if (currentToken.type == TOKEN_KEYWORD_ELSE) {
            advance();  // consome 'else'
            char* labelAfterElse = novaLabel();  // Label para fim do else
            gerarGoto(labelAfterElse);            // Salto após o else
            gerarLabel(labelFim);                 // Label do início do else
            parseCmd();                          // Comando else
            gerarLabel(labelAfterElse);          // Label após o else
            free(labelAfterElse);
        } else {
            // Se não houver else, finaliza com labelFim
            gerarLabel(labelFim);
        }

        // Libera memória alocada para as strings de labels e expressões
        free(esq);
        free(dir);
        free(labelThen);
        free(labelFim);
    }
    else if (currentToken.type == TOKEN_KEYWORD_WHILE) {
        // Comando while
        printf("[CMD] Reconhecido comando 'while'\n");
        advance();  // consome 'while'

        // Cria labels para início da condição, corpo do loop e fim do loop
        char* labelInicio = novaLabel();
        char* labelCorpo  = novaLabel();
        char* labelFim    = novaLabel();

        gerarLabel(labelInicio); // Marca início da condição

        parseEat(TOKEN_LPAREN);  // espera e consome '('

        // Analisa a condição do while: expr op expr
        char* esq = parseExprSimp();
        char op[4];
        strcpy(op, currentToken.lexeme);
        advance();
        char* dir = parseExprSimp();

        parseEat(TOKEN_RPAREN);  // espera e consome ')'

        // Geração de código para condição: carrega valores, subtrai e gera saltos
        gerarLoad(esq);
        gerarLoad(dir);
        gerarSub();

        // Condições para saltos condicionais no loop, semelhante ao if
        if (strcmp(op, ">") == 0 || strcmp(op, "!=") == 0) {
            gerarGotoTrue(labelCorpo);
            gerarGoto(labelFim);
        } else if (strcmp(op, "<") == 0 || strcmp(op, "==") == 0) {
            gerarGotoFalse(labelCorpo);
            gerarGoto(labelFim);
        } else if (strcmp(op, ">=") == 0) {
            gerarGotoFalse(labelFim);
            gerarGoto(labelCorpo);
        } else if (strcmp(op, "<=") == 0) {
            gerarGotoTrue(labelFim);
            gerarGoto(labelCorpo);
        } else {
            // Erro para operador desconhecido
            fprintf(stderr, "[ERRO] Operador relacional desconhecido: %s\n", op);
            exit(1);
        }

        // Corpo do loop
        gerarLabel(labelCorpo);
        parseCmd();

        // Volta para reavaliar a condição
        gerarGoto(labelInicio);

        // Marca o fim do loop
        gerarLabel(labelFim);

        // Libera memória alocada
        free(esq);
        free(dir);
        free(labelInicio);
        free(labelCorpo);
        free(labelFim);

    } else if (currentToken.type == TOKEN_KEYWORD_FOR) {
        // Comando for
        printf("[CMD] Reconhecido comando 'for'\n");
        advance();  // consome 'for'

        parseEat(TOKEN_LPAREN);  // consome '('

        // Inicialização (ex: i = 0;)
        if (currentToken.type == TOKEN_ID) {
            parseAtrib();
        }
        parseEat(TOKEN_SEMICOLON);

        // Condição do for (expressão)
        if (currentToken.type != TOKEN_SEMICOLON) {
            parseExpr();
        }
        parseEat(TOKEN_SEMICOLON);

        // Incremento (ex: i = i + 1;)
        if (currentToken.type == TOKEN_ID) {
            parseAtrib();
        }
        parseEat(TOKEN_RPAREN);  // consome ')'

        // Corpo do for
        parseCmd();

    } else if (currentToken.type == TOKEN_KEYWORD_RETURN) {
        // Comando return
        printf("[CMD] Reconhecido comando 'return'\n");
        advance();  // consome 'return'

        // Se houver expressão para retornar (ex: return 10;)
        if (currentToken.type != TOKEN_SEMICOLON) {
            parseExpr();

            // Obtém resultado da expressão para código
            const char* resultado = getUltimoResultado();

            verificarReturnComValor(); // Verifica regras semânticas
            gerarRetorno(resultado);   // Gera código de retorno
        } else {
            // return sem valor (ex: return;)
            verificarReturnSemValor();
            gerarRetorno(NULL);
        }

        parseEat(TOKEN_SEMICOLON);

    } else if (currentToken.type == TOKEN_LBRACE) {
        // Comando bloco composto (conjunto de comandos entre chaves)
        printf("[CMD] Bloco composto reconhecido\n");
        advance();  // consome '{'

        // Analisa comandos dentro do bloco até encontrar '}'
        while (currentToken.type != TOKEN_RBRACE && currentToken.type != TOKEN_EOF) {
            parseCmd();
        }
        parseEat(TOKEN_RBRACE);  // consome '}'

    } else if (currentToken.type == TOKEN_SEMICOLON) {
        // Comando vazio (ponto e vírgula sozinho)
        printf("[CMD] Comando vazio reconhecido\n");
        advance();

    } else if (currentToken.type == TOKEN_ID) {
        // Caso o token atual seja um identificador, pode ser atribuição ou chamada de função

        // Lookahead para decidir qual caso é (atribuição ou chamada)
        Token lookahead = getNextToken();
        ungetToken(lookahead);  // devolve o token para ser processado depois

        if (lookahead.type == TOKEN_ASSIGN || lookahead.type == TOKEN_LBRACK) {
            // Atribuição (ex: a = 10; ou a[0] = 10;)
            parseAtrib();
            parseEat(TOKEN_SEMICOLON);
            return;

        } else if (lookahead.type == TOKEN_LPAREN) {
            // Chamada de função (ex: func(...);)
            printf("[CMD] Chamada de função reconhecida: %s\n", currentToken.lexeme);

            verificarUsoDeFuncaoComoComando(currentToken.lexeme);

            advance();  // consome id (nome da função)
            parseEat(TOKEN_LPAREN);

            // Analisa argumentos da chamada, se existirem
            if (currentToken.type != TOKEN_RPAREN) {
                parseExpr();

                while (currentToken.type == TOKEN_COMMA) {
                    advance();
                    parseExpr();
                }
            }

            parseEat(TOKEN_RPAREN);
            parseEat(TOKEN_SEMICOLON);
            return;
        } else {
            // Caso inesperado, erro de sintaxe
            parseError("Identificador inesperado — esperada atribuição ou chamada de função");
        }

    } else {
        // Caso nenhum comando reconhecido, erro
        printf("[CMD] Comando inválido ou não tratado: token '%s'\n", currentToken.lexeme);
        parseError("Comando não reconhecido");
    }
}

// Analisa uma atribuição
// atrib ::= id [ '[' expr ']' ] = expr
void parseAtrib() {

    // Verifica se o token atual é um identificador (nome da variável)
    if (currentToken.type != TOKEN_ID) {
        parseError("Esperado identificador no início da atribuição");
        return;  // Sai da função pois não é atribuição válida
    }

    // Copia o nome da variável para uma string local
    char nomeVar[128];  // pode ser MAX_TOKEN_LEN, se definido
    strcpy(nomeVar, currentToken.lexeme);

    // Verifica se a variável foi declarada anteriormente (semântica)
    verificarVariavelDeclarada(nomeVar);

    // Inicia a lógica para atribuição, prepara estado interno se necessário
    iniciarAtribuicao(nomeVar);

    // Avança para próximo token após o identificador
    advance();

    bool isVetor = false;      // flag que indica se é atribuição em vetor
    char* tempIndex = NULL;    // expressão que indica o índice no vetor (se houver)

    // Se encontrar '[', indica que a atribuição é a um elemento de vetor
    if (currentToken.type == TOKEN_LBRACK) {
        isVetor = true;        // marca que é vetor
        advance();             // consome '['
        tempIndex = parseExpr();  // parseia a expressão do índice
        parseEat(TOKEN_RBRACK);   // espera e consome ']'
    }

    // Espera e consome o símbolo '=' da atribuição
    parseEat(TOKEN_ASSIGN);

    // Parseia a expressão que representa o valor a ser atribuído
    char* tempValor = parseExpr();

    // Verifica semântica: se o tipo da expressão é compatível com a variável
    verificarTipoExpr();

    // Gera código para a atribuição
    if (isVetor) {
        // Atribuição a um elemento do vetor
        gerarComando("STORE %s[%s] %s", nomeVar, tempIndex, tempValor);
    } 

    printf("[ATRIB] Atribuição completa reconhecida\n");
}

// Analisa uma expressão com possível operador relacional
// Retorna o nome temporário da variável onde o resultado da expressão será armazenado
// expr ::= expr_simp [ op_rel  expr_simp ] 
char* parseExpr() {

    // Analisa a expressão simples à esquerda (expr_simp)
    char* tempEsq = parseExprSimp();

    // Obtém o tipo da expressão simples esquerda (ex: "int", "char")
    const char* tipoAntesOperadorRel = getTipoExpressao();

    // Verifica se o próximo token é um operador relacional
    if (currentToken.type == TOKEN_EQ || currentToken.type == TOKEN_NEQ ||
        currentToken.type == TOKEN_LT || currentToken.type == TOKEN_GT ||
        currentToken.type == TOKEN_LEQ || currentToken.type == TOKEN_GEQ) {
        
        int operador = currentToken.type;  // guarda o operador relacional
        advance();                         // consome o operador

        // Analisa a expressão simples à direita (expr_simp)
        char* tempDir = parseExprSimp();

        // Obtém o tipo da expressão simples direita
        const char* tipoDepoisOperadorRel = getTipoExpressao();

        // Verifica se os operandos são do tipo int ou char (não aceita bool para operadores relacionais)
        if (!(strcmp(tipoAntesOperadorRel, "int") == 0 || strcmp(tipoAntesOperadorRel, "char") == 0) ||
            !(strcmp(tipoDepoisOperadorRel, "int") == 0 || strcmp(tipoDepoisOperadorRel, "char") == 0)) {
            fprintf(stderr, "[ERRO SEMÂNTICO] Operadores relacionais requerem operandos do tipo int ou char (não bool)\n");
            setTipoExpressao("erro");   // marca erro no tipo da expressão atual
        } else {
            registrarTipoRelacional();  // registra que o tipo da expressão é relacional (booleano)
        }

        // Converte o token do operador para string correspondente (ex: "==", "<")
        const char* opStr = NULL;
        switch (operador) {
            case TOKEN_EQ:  opStr = "=="; break;
            case TOKEN_NEQ: opStr = "!="; break;
            case TOKEN_LT:  opStr = "<";  break;
            case TOKEN_GT:  opStr = ">";  break;
            case TOKEN_LEQ: opStr = "<="; break;
            case TOKEN_GEQ: opStr = ">="; break;
        }

        // Verifica se o operador foi reconhecido corretamente
        if (opStr == NULL) {
            fprintf(stderr, "[ERRO] Operador relacional inválido!\n");
        }

        // Cria um novo temporário para armazenar o resultado da comparação
        char* tempResult = novoTemporario();

        // Gera o código intermediário para a comparação relacional
        gerarComparacaoRelacional(opStr, tempEsq, tempDir, tempResult);

        // Armazena o resultado atual para uso posterior
        setUltimoResultado(tempResult);

        // Retorna o temporário com o resultado da expressão relacional
        return tempResult;
    }

    // Se não houver operador relacional, retorna o resultado da expressão simples
    setUltimoResultado(tempEsq);
    return tempEsq;
}

// Analisa uma expressão simples que pode começar com operador unário (+ ou -)
// seguida por um termo, e depois pode ter uma sequência de operadores binários
// (+, -, ||) seguidos de termos
// Retorna o nome temporário onde o resultado da expressão será armazenado
// expr_simp ::= [+ | – ] termo {(+ | – | ||) termo} 
char* parseExprSimp() {
    // Tratamento de operador unário + ou - (ignorado no momento)
    if (currentToken.type == TOKEN_PLUS || currentToken.type == TOKEN_MINUS) {
        advance(); // operador unário
        // opcional: implementar comportamento unário aqui
    }

    // Analisa o primeiro termo da expressão simples
    char* tempEsq = parseTermo();

    // Obtém o tipo do termo analisado, para controle semântico/tipo
    const char* tipoAnterior = getTipoExpressao();

    // Enquanto o próximo token for operador +, - ou ||, continua analisando
    while (currentToken.type == TOKEN_PLUS || 
           currentToken.type == TOKEN_MINUS || 
           currentToken.type == TOKEN_OR) {
        
        int operador = currentToken.type;  // guarda o operador atual
        advance();                         // consome o operador

        // Analisa o próximo termo (lado direito da operação)
        char* tempDir = parseTermo();
        const char* tipoDir = getTipoExpressao();

        if (operador == TOKEN_OR) {
            // Operação lógica OR '||' requer operandos do tipo bool
            if (strcmp(tipoAnterior, "bool") != 0 || strcmp(tipoDir, "bool") != 0) {
                fprintf(stderr, "[ERRO SEMÂNTICO] Operador || requer bool\n");
                setTipoExpressao("erro");   // marca erro de tipo
            } else {
                registrarTipoLogico();  // registra tipo lógico para a expressão atual
            }
            // Geração de código simplificada para operador OR
            gerarLoad(tempEsq);  // carrega valor da esquerda
            gerarLoad(tempDir);  // carrega valor da direita
            gerarComando("OR");  // gera comando customizado OR (bit a bit ou lógico)
        } else {
            // Operação aritmética: soma ou subtração
            // Determina o tipo dominante da operação entre os dois operandos
            const char* tipoDominante = tipoDominanteAritmetico(tipoAnterior, tipoDir);

            // Atualiza o tipo da expressão para o tipo dominante
            setTipoExpressao(tipoDominante);

            // Gera código intermediário para carregar operandos
            gerarLoad(tempEsq);
            gerarLoad(tempDir);

            if (operador == TOKEN_PLUS) {
                gerarComando("SOMA");  // gera comando customizado para soma
            } else { // operador é TOKEN_MINUS
                gerarComando("SUB");   // gera comando customizado para subtração
            }
        }

        // Cria um temporário para armazenar o resultado da operação
        char* tempResult = novoTemporario();

        // Gera comando para armazenar o resultado no temporário criado
        gerarStore(tempResult);

        // Atualiza o lado esquerdo para o próximo loop com o resultado atual
        tempEsq = tempResult;

        // Atualiza o tipo anterior para o tipo da expressão atual (pode ter mudado)
        tipoAnterior = getTipoExpressao();
    }

    // Retorna o nome do temporário onde o resultado final está armazenado
    return tempEsq;
}

// termo ::= fator {(* | / | &&)  fator} 
// Analisa e traduz um termo da expressão
// Realiza verificações semânticas e gera código intermediário com LOAD, MULT, DIV, AND
char* parseTermo() {
    // Analisa o primeiro fator e armazena seu valor temporário
    char* tempEsq = parseFator();
    const char* tipoAnterior = getTipoExpressao();

    // Enquanto houver operadores multiplicativos ou lógicos (AND)
    while (currentToken.type == TOKEN_MUL || 
           currentToken.type == TOKEN_DIV || 
           currentToken.type == TOKEN_AND) {
        
        int operador = currentToken.type;
        advance();  // Consome o operador

        // Analisa o fator à direita do operador
        char* tempDir = parseFator();
        const char* tipoAtual = getTipoExpressao();

        if (operador == TOKEN_AND) {
            // Verifica se ambos os operandos são booleanos
            if (strcmp(tipoAnterior, "bool") != 0 || strcmp(tipoAtual, "bool") != 0) {
                fprintf(stderr, "[ERRO SEMÂNTICO] Operador && requer bool\n");
                setTipoExpressao("erro");
            } else {
                registrarTipoLogico(); // Define tipo da expressão como bool
            }

            // Geração de código para operação lógica AND
            gerarLoad(tempEsq);
            gerarLoad(tempDir);
            gerarComando("AND");
        } else {
            // Para MULT e DIV, calcula o tipo dominante (ex: int vs float → float)
            setTipoExpressao(tipoDominanteAritmetico(tipoAnterior, tipoAtual));

            // Geração do código no formato LOAD MULT DIV
            gerarLoad(tempEsq);
            gerarLoad(tempDir);

            if (operador == TOKEN_MUL) {
                gerarComando("MULT");
            } else { // operador == TOKEN_DIV
                gerarComando("DIV");
            }
        }

        // Armazena o resultado em um novo temporário
        char* tempResult = novoTemporario();
        gerarStore(tempResult);

        tempEsq = tempResult;
        tipoAnterior = getTipoExpressao();  // Atualiza tipoAnterior para próximo loop
    }

    return tempEsq; // Retorna o nome do temporário que contém o resultado do termo
}

// Analisa e traduz um fator da gramática
// fator ::= id[...] | constantes | chamada | (!fator)
char* parseFator() {

    if (currentToken.type == TOKEN_ID) {
        Token id = currentToken;
        advance();

        // Verifica se é chamada de função: id(...)
        if (currentToken.type == TOKEN_LPAREN) {
            advance();
            char* args[10];
            int qtdArgs = 0;

            // Lê os argumentos, se houver
            if (currentToken.type != TOKEN_RPAREN) {
                args[qtdArgs++] = parseExpr();
                while (currentToken.type == TOKEN_COMMA) {
                    advance();
                    args[qtdArgs++] = parseExpr();
                }
            }

            parseEat(TOKEN_RPAREN);

            // Gera código da chamada e retorna temp com resultado
            char* tempResult = novoTemporario();
            gerarChamadaFuncao(tempResult, id.lexeme, args, qtdArgs);
            setTipoExpressao("int"); // pode variar conforme tipo da função
            return tempResult;

        } else {
            // Se não é chamada, então é uso de variável (id ou vetor)
            verificarVariavelDeclarada(id.lexeme);
            char* temp = novoTemporario();
            gerarComando("%s = %s", temp, id.lexeme);
            setTipoExpressao("int"); // pode variar conforme tipo da variável
            return temp;
        }

    } else if (currentToken.type == TOKEN_INTCON) {
        // Constante inteira
        char* temp = novoTemporario();

        if (!temp || strlen(currentToken.lexeme) == 0) {
            printf("[ERRO] Ponteiro NULL detectado!\n");
            exit(1);
        }

        if (currentToken.lexeme[0] == '\0') {
            fprintf(stderr, "[ERRO] Lexema vazio detectado (token malformado)\n");
            exit(1);
        }

        gerarComando("%s = %s", temp, currentToken.lexeme);

        setTipoExpressao("int");
        advance();
        return temp;

    } else if (currentToken.type == TOKEN_CHARCON) {
        // Constante char
        char* temp = novoTemporario();
        gerarComando("%s = '%s'", temp, currentToken.lexeme);
        setTipoExpressao("char");
        advance();
        return temp;
    } else if (currentToken.type == TOKEN_REALCON) {
        // Constante float
        char* temp = novoTemporario();
        gerarComando("%s = %s", temp, currentToken.lexeme);
        setTipoExpressao("float");
        advance();
        return temp;

    } else if (currentToken.type == TOKEN_BOOLCON) {
        // Constante bool
        char* temp = novoTemporario();
        gerarComando("%s = %s", temp, currentToken.lexeme);
        setTipoExpressao("bool");
        advance();
        return temp;
    } else if (currentToken.type == TOKEN_LPAREN) {
        // Expressão entre parênteses
        advance();
        char* temp = parseExpr();
        parseEat(TOKEN_RPAREN);
        return temp;

    } else {
        // Qualquer outro caso é erro
        parseError("Fator inválido");
        return NULL;
    }
}

// ==============================
// Funções auxiliares de análise
// ==============================

// Analisa a primeira variável de uma declaração de variáveis.
// Exemplo: na linha "int x, y, z;", essa função trata a variável "x".
// Verifica se é vetor (ex: int v[10]) e registra no escopo correto (global ou local).
void parseDeclVarPrimeiro(const char* tipo, Escopo escopo) {
    // Verifica se o token atual é um identificador (nome da variável)
    if (currentToken.type != TOKEN_ID) {
        parseError("Esperado identificador na declaração de variável");
    }

    char nome[256];
    int isVetor = 0;     // Flag para indicar se é vetor
    int tamanho = 1;     // Tamanho padrão (1) para variáveis escalares

    // Copia o nome do identificador para a variável local
    strncpy(nome, currentToken.lexeme, sizeof(nome));
    nome[sizeof(nome) - 1] = '\0';
    advance(); // Consome o identificador

    printf("[DECL_VAR] Reconhecida variável: %s\n", nome);

    // Verifica se a variável é um vetor (possui colchetes [])
    if (currentToken.type == TOKEN_LBRACK) {
        advance();
        if (currentToken.type == TOKEN_INTCON) {
            isVetor = 1;    // Marca como vetor
            tamanho = atoi(currentToken.lexeme);    // Converte o tamanho do vetor
            printf("[DECL_VAR] Vetor com tamanho: %s\n", currentToken.lexeme);
            advance();
            parseEat(TOKEN_RBRACK);   // Espera fechar o colchete
        } else {
            parseError("Esperado número inteiro dentro dos colchetes");
        }
    }

    // Registra a variável no escopo apropriado
    if (escopo == ESC_GLOBAL)
        registrarVariavelGlobal(tipo, nome, isVetor, tamanho);
    else
        registrarVariavelLocal(tipo, nome, isVetor, tamanho);
}

// Demais variáveis após vírgula (ex: int x, y[10], z;)
// Esta função trata as variáveis que aparecem depois da primeira em uma declaração múltipla.
void parseDeclVarResto(const char* tipo, Escopo escopo) {

    // Enquanto houver vírgulas, há mais variáveis na mesma declaração
    while (currentToken.type == TOKEN_COMMA) {
        advance(); // consome ','

        // Após a vírgula deve vir um identificador
        if (currentToken.type != TOKEN_ID) {
            parseError("Esperado identificador após ','");
        }

        // Inicializa dados da variável
        char nome[256];
        int isVetor = 0;
        int tamanho = 1;

        // Copia o nome do identificador
        strncpy(nome, currentToken.lexeme, sizeof(nome));
        nome[sizeof(nome) - 1] = '\0';
        advance(); // consome o identificador

        printf("[DECL_VAR] Reconhecida variável extra: %s\n", nome);

        // Verifica se é vetor: identificador seguido por colchetes
        if (currentToken.type == TOKEN_LBRACK) {
            advance();  // consome '['
            if (currentToken.type == TOKEN_INTCON) {
                isVetor = 1;
                tamanho = atoi(currentToken.lexeme);    // converte tamanho para int
                printf("[DECL_VAR] Vetor de tamanho: %s\n", currentToken.lexeme);
                advance();  // consome o número
                parseEat(TOKEN_RBRACK); // consome ']'
            } else {
                parseError("Esperado número inteiro dentro dos colchetes");
            }
        }

        // Registra a variável no escopo apropriado (global ou local)
        if (escopo == ESC_GLOBAL)
            registrarVariavelGlobal(tipo, nome, isVetor, tamanho);
        else
            registrarVariavelLocal(tipo, nome, isVetor, tamanho);

    }
}

// Analisa e registra um parâmetro formal de função.
// Tipo (id | &id | id[])
void parseTipoParam() {
    if (!isTipo(currentToken.type)) {
        parseError("Esperado tipo (int, char, float, bool) no parâmetro");
    }

    char tipoStr[10];                  // ← Captura o tipo ANTES de consumir
    obterTipoString(tipoStr);

    if (numParamsTemp < MAX_PARAMS_FUNCAO) {
        strncpy(tiposParamsTemp[numParamsTemp], tipoStr, sizeof(tiposParamsTemp[numParamsTemp]));
        tiposParamsTemp[numParamsTemp][sizeof(tiposParamsTemp[numParamsTemp]) - 1] = '\0';
        numParamsTemp++;
    } else {
        parseError("Número excessivo de parâmetros na função");
    }

    advance(); // consome o tipo

    // Verifica se é '&' (um único token do tipo TOKEN_AND)
    int porReferencia = 0;
    if (currentToken.type == TOKEN_BITAND) {
        porReferencia = 1;
        advance(); // consome '&'
    }

    // Agora deve vir um identificador
    if (currentToken.type != TOKEN_ID) {
        parseError("Esperado identificador no parâmetro");
    }

    char nome[256];
    strncpy(nome, currentToken.lexeme, sizeof(nome));
    nome[sizeof(nome) - 1] = '\0';

    // Verifica se já existe parâmetro com mesmo nome
    verificarParametroRepetido(nome);  // ← ESTA LINHA É A NOVA ADIÇÃO

    // Armazena o nome após checar
    strncpy(nomesParamsTemp[numParamsTemp], nome, sizeof(nomesParamsTemp[numParamsTemp]));
    nomesParamsTemp[numParamsTemp][sizeof(nomesParamsTemp[numParamsTemp]) - 1] = '\0';

    numParamsTemp++; // só incrementa aqui, após nome e tipo armazenados

    advance(); // consome ID

    // Verifica se é vetor
    int isVetor = 0;
    if (currentToken.type == TOKEN_LBRACK) {
        advance();
        parseEat(TOKEN_RBRACK);
        isVetor = 1;
    }

    // ao chamar registrarParametro
    if (porReferencia) {
        registrarParametro(tipoStr, nome, CLASSE_PARAM, ESC_LOCAL, 1);
    } else if (isVetor) {
        registrarParametro(tipoStr, nome, CLASSE_VETOR, ESC_LOCAL, 1);
    } else {
        registrarParametro(tipoStr, nome, CLASSE_PARAM, ESC_LOCAL, 1);
    }
}

// Reconhece e processa uma **lista de variáveis** do mesmo tipo
void parseDeclVarLista(const char* tipo, Escopo escopo) {
    // A primeira variável já foi lida, processa ela
    parseDeclVar(tipo, ESC_GLOBAL); // ← ERRADO: deveria ser 'escopo' e não 'ESC_GLOBAL'

    // Enquanto houver vírgulas, há mais variáveis na lista
    while (currentToken.type == TOKEN_COMMA) {
        advance(); // consome ','
        parseDeclVar(tipo, ESC_GLOBAL); // ← MESMO ERRO: usar 'escopo' passado por parâmetro
    }
}

// ==============================
// Utilitários de parsing
// ==============================

// Verifica se o token fornecido representa um tipo primitivo válido em Cshort.
int isTipo(TokenType t) {
    return t == TOKEN_KEYWORD_INT  ||  // tipo inteiro
           t == TOKEN_KEYWORD_CHAR ||  // tipo caractere
           t == TOKEN_KEYWORD_FLOAT || // tipo ponto flutuante
           t == TOKEN_KEYWORD_BOOL;    // tipo booleano
}

// Verifica se o token fornecido pode iniciar um comando válido em Cshort
int isComandoInicio(TokenType t) {
    return t == TOKEN_KEYWORD_IF      ||  // if (...)
           t == TOKEN_KEYWORD_WHILE   ||  // while (...)
           t == TOKEN_KEYWORD_RETURN  ||  // return ...
           t == TOKEN_LBRACE          ||  // bloco de comandos { ... }
           t == TOKEN_ID              ||  // comando que começa com identificador
           t == TOKEN_SEMICOLON;          // comando nulo (;)
}

// ==============================
// SIMBOLOS FUNC AUXILIARES
// ==============================

// Copia o nome textual do tipo do token atual para a string 'dest'.
// Usado para registrar o tipo de uma variável ou parâmetro com base no token.void obterTipoString(char* dest) {
void obterTipoString(char* dest) {
    switch (currentToken.type) {
        case TOKEN_KEYWORD_INT:   strcpy(dest, "int"); break;
        case TOKEN_KEYWORD_FLOAT: strcpy(dest, "float"); break;
        case TOKEN_KEYWORD_CHAR:  strcpy(dest, "char"); break;
        case TOKEN_KEYWORD_BOOL:  strcpy(dest, "bool"); break;
        default: strcpy(dest, "???"); break;  // tipo não reconhecido
    }
}

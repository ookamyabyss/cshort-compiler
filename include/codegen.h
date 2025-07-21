#ifndef CODEGEN_H
#define CODEGEN_H

// ============================
// INICIALIZAÇÃO / FINALIZAÇÃO
// ============================

// Abre o arquivo onde o código intermediário será escrito
void iniciarCodegen(const char* nomeArquivo);

// Fecha o arquivo de código intermediário
void finalizarCodegen();

// ============================
// TEMPORÁRIOS E LABELS
// ============================

// Gera um novo nome de temporário único (ex: T0, T1...)
char* novoTemporario();

// Gera um novo nome de label único (ex: LABEL0, LABEL1...)
char* novaLabel();

// ============================
// FUNÇÕES DE BAIXO NÍVEL
// ============================

// Escreve um comando formatado no arquivo de saída
void gerarComando(const char* formato, ...);

// ============================
// FUNÇÕES DE COMANDO DIRETO
// ============================

// Gera um comando LOAD (carrega valor para o topo da pilha)
void gerarLoad(const char* varOuTemp);

// Gera um comando STORE (guarda o topo da pilha na variável)
void gerarStore(const char* varOuTemp) ;

// Simula um store em vetor: var[index] = valor
void gerarStoreArray(const char* var, int tempIndex, int tempValor);

// Empilha valor diretamente
void gerarPush(const char* valor);

// Operações aritméticas básicas
void gerarAdd();

void gerarSub();

// Desvios incondicionais e condicionais
void gerarGoto(const char* label);

void gerarGotoTrue(const char* label);

void gerarGotoFalse(const char* label);

// Define um label no código
void gerarLabel(const char* label);

// ============================
// FUNÇÕES DE BLOCO
// ============================

// Início de uma função
void gerarInicioFuncao(const char* nomeFunc);

// Fim de uma função
void gerarFimFuncao();

// Declaração de variável (ex: VAR x : int)
void gerarDeclaracaoVar(const char* nomeVar, const char* tipo);

// Gera um comando vazio (;)
void gerarComandoVazio();

// Gera LOAD + STORE para uma atribuição simples
void gerarRetorno(const char* expr);

// Gera LOAD + STORE para uma atribuição simples
void gerarAtribuicao(const char* destino, const char* fonte);

// Exibe store direto (usado para vetores ou debug)
void gerarStoreDireto(const char* nomeVar, const char* temp);

// ============================
// COMPARAÇÕES RELACIONAIS
// ============================

// Gera código para comparações relacionais (==, !=, >, <, >=, <=)
void gerarComparacaoRelacional(const char* op, const char* esq, const char* dir, const char* destino);

// ============================
// CONTROLE DE FLUXO (IF, WHILE, FOR)
// ============================

// Desvio para label se condição for falsa
void gerarIfFalso(const char* condicao, const char* labelFalsa);

// Desvio para label se condição for verdadeira
void gerarIf(const char* condicao, const char* labelVerdadeira);

// Traduz if-else completo com blocos de código
void gerarIfElseComBlocos(const char* condicao, void (*blocoIf)(void), void (*blocoElse)(void));

// Gera estrutura de repetição while (com corpo e condição)
void gerarWhileComBlocos(void (*blocoCondicao)(void), void (*blocoCorpo)(void));

// Gera estrutura de repetição for (init; cond; iter; corpo)
void gerarForComBlocos(void (*blocoInit)(void), void (*blocoCond)(void), void (*blocoIter)(void), void (*blocoCorpo)(void));

// ============================
// CHAMADAS DE FUNÇÃO
// ============================

// Gera chamada de função com parâmetros e atribuição do retorno
void gerarChamadaFuncao(const char* destino, const char* nomeFunc, char* args[], int qtdArgs);

// ============================
// SUPORTE A TEMPORÁRIOS
// ============================

// Armazena o último temporário gerado (útil para expressões compostas)
void setUltimoResultado(const char* temp);

// Recupera o último temporário salvo
const char* getUltimoResultado(void);


#endif

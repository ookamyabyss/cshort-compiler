<h1 align="center">⚙️ Cshort Compiler</h1> <h3 align="center">🔍 Analisador Léxico + 🧠 Analisador Sintático — Versão 1.0</h3> <p align="center"> <img src="https://img.shields.io/badge/status-stable-brightgreen" alt="status"> <img src="https://img.shields.io/badge/version-1.0-blue" alt="version"> <img src="https://img.shields.io/badge/AFD-2.3-lightgrey" alt="afd"> </p>
🚀 Visão Geral
Este projeto implementa um Analisador Léxico e um Analisador Sintático para uma linguagem estilo C, chamada Cshort.
É parte do projeto de compiladores, seguindo uma gramática definida com suporte a declarações, comandos, expressões e funções.

## ⚙️ Pré-requisitos

- **MSYS2 instalado**
- Terminal: **MSYS2 MinGW 64-bit**
- Pacotes obrigatórios (instalar no terminal MSYS2):

Atualiza o sistema (pode pedir para reiniciar)

```bash
    pacman -Su    
```

Depois, instale o compilador e ferramentas:

```bash
pacman -S base-devel mingw-w64-x86_64-toolchain
```
🛠️ Compilação

No terminal MSYS2 MinGW 64-bit, vá até a pasta do projeto:

```bash
cd /c/Users/seu_usuario/caminho/para/cshort-compiler
```

Isso irá compilar o projeto e gerar o executável:

```bash
make
```

```bash
./build/lexer 'nome do arq'

```

<details> <summary><strong>📜 Gramática — Cshort v1.0 (clique para expandir)</strong></summary>

// prog ::= { decl ';' | func } 

// decl ::= tipo decl_var { ',' decl_var}  
//        | tipo id '(' tipos_param')' { ',' id '(' tipos_param')' } 
//        | void id '(' tipos_param')' { ',' id '(' tipos_param')' }

// decl_var ::= id [ '[' intcon ']' ]

// tipo ::= char | int | float | bool

// tipos_param ::= void 
//               | tipo (id | &id | id '[' ']') { ',' tipo (id | &id | id '[' ']') }

// func ::= tipo id '(' tipos_param ')' '{' { tipo decl_var { ',' decl_var } ';' } { cmd } '}'
//        | void id '(' tipos_param ')' '{' { tipo decl_var { ',' decl_var } ';' } { cmd } '}'

// cmd ::= if '(' expr ')' cmd [ else cmd ] 
//       | while '(' expr ')' cmd 
//       | for '(' [ atrib ] ';' [ expr ] ';' [ atrib ] ')' cmd 
//       | return [ expr ] ';' 
//       | atrib ';' 
//       | id '(' [expr { ',' expr } ] ')' ';' 
//       | '{' { cmd } '}' 
//       | ';' 

// atrib ::= id [ '[' expr ']' ] = expr

// expr ::= expr_simp [ op_rel expr_simp ]

// expr_simp ::= [+ | - ] termo {(+ | - | ||) termo}

// termo ::= fator {(* | / | & ) fator}

// fator ::= id [ '[' expr ']' ] 
//         | intcon | realcon | charcon 
//         | id '(' [expr { ',' expr } ] ')' 
//         | '(' expr ')' 
//         | '!' fator

</details>

🧮 AFD - Autômato Finito Determinístico
📅 Versão 2.3 - 21/06/2025

<p align="center"> <img src="./assets/cshort_afd.png" alt="AFD - Cshort" width="600"/> </p>

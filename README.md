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

📜 Gramática — Cshort v1.0
<details> <summary><strong>🎯 Regras principais (clique para expandir)</strong></summary>

// prog ::= { decl ';' | func } 

// decl ::= tipo decl_var { ',' decl_var}  
//        | tipo id '(' tipos_param')' { ',' id '(' tipos_param')' } 
//        | void id '(' tipos_param')' { ',' id '(' tipos_param')' }

// decl_var ::= id [ '[' intcon ']' ]

![Cshort](./assets/cshort_afd.png)

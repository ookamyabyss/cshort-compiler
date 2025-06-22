<h1 align="center">⚙️ Cshort Compiler</h1> <h3 align="center">🔍 Analisador Léxico + 🧠 Analisador Sintático — Versão 1.0</h3> <p align="center"> <img src="https://img.shields.io/badge/status-stable-brightgreen" alt="status"> <img src="https://img.shields.io/badge/version-1.0-blue" alt="version"> <img src="https://img.shields.io/badge/AFD-2.3-lightgrey" alt="afd"> </p>
---

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

![Cshort](./assets/cshort_afd.png)

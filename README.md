**Cshort Compiler** - Analisador Léxico vers 1.0 

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
build/lexer.exe
```

![Cshort](./assets/cshort_afd.png)

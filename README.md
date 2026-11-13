<h1 align="center">🍕 PizzAlign</h1>

> Projeto desenvolvido para a disciplina de **Programação Imperativa e Funcional (PIF)**. Consiste em um jogo casual dinâmico construído em **C**, com interface gráfica em janela utilizando a biblioteca **raylib**, aplicando conceitos de estruturas, ponteiros, matrizes, alocação dinâmica, listas encadeadas e manipulação de arquivos.

## Sobre

**PizzAlign** mistura a lógica de quebra-cabeças com o dinamismo dos jogos de queda de blocos. O design e as mecânicas foram inspirados em diversos títulos:

- **Tetris:** A gravidade contínua, o formato de tabuleiro em matriz e a necessidade de limpar o espaço antes que transborde.
- **Half Moon (Google):** A ideia central de juntar frações para formar um círculo completo (neste caso, uma pizza).
- **Suika Game (Watermelon Game):** A mecânica de colisão e fusão (evolução) baseada em compatibilidade.
- **Overcooked:** O visual, com texturas de ingredientes, balcões de madeira e animações.
- **Make7 / 2048:** Os blocos arredondados com números centralizados, cores distintas para cada valor de fatia e efeitos de escala ao fundir peças.
- **Tetris Effect:** O feedback visual e sonoro que torna cada combinação satisfatória, com efeito de partículas ao fechar uma pizza inteira.

## Regras

- **Gravidade e Queda:** Fatias de pizza com diferentes valores (de 1 a 5 pedaços) caem do topo da tela em direção ao fundo do tabuleiro.
- **Sistema de Colisão e Fusão:**
  - Quando uma fatia cai sobre outra, o jogo verifica a soma de seus pedaços.
  - Se a soma for **menor ou igual a 6**, as duas peças se fundem, criando uma nova fatia com o valor total.
  - Se a soma for **maior que 6**, a fusão falha e a peça apenas se empilha sobre a anterior.
- **Pontuação:** O objetivo é fazer fusões que resultem no valor **6 (pizza inteira)**. Quando isso acontece, a pizza é entregue, desaparece do tabuleiro liberando espaço e você ganha pontos.
- **Fim de Jogo:** A partida termina se a pilha de fatias atingir o limite superior do tabuleiro.
- **Top Score:** O jogo salva automaticamente a maior pontuação no arquivo `top_score.txt`.

## Como Jogar

Você controla a peça apenas enquanto ela está caindo. Utilize o teclado para gerenciar os movimentos:

| Tecla | Ação |
| :--- | :--- |
| `A` / `Seta para Esquerda` | Move a fatia em queda para a esquerda |
| `D` / `Seta para Direita` | Move a fatia em queda para a direita |
| `S` / `Seta para Baixo` | Acelera a queda da fatia (*soft drop*) |
| `M` / `Menu` | Volta ao menu do jogo |
| `Q` / `ESC` | Sai do jogo |

## Execução

### Pré-requisitos

Para executar este jogo, você precisará de um ambiente baseado em Unix (Linux, macOS ou WSL no Windows) com as seguintes ferramentas instaladas:

- `gcc` (Compilador C)
- `make` (Ferramenta de automação de compilação)
- `raylib` (Biblioteca gráfica)

### Instalação da raylib

**Linux (Debian/Ubuntu):**
```bash
sudo apt update
sudo apt install build-essential libraylib-dev
```
> Caso `libraylib-dev` não esteja disponível na sua distribuição, siga o guia de compilação da raylib a partir do código-fonte.

**macOS (Homebrew):**
```bash
brew install raylib
```

**Windows (MSYS2 / terminal MinGW64):**
```bash
pacman -S mingw-w64-x86_64-gcc mingw-w64-x86_64-make mingw-w64-x86_64-raylib
```
> Alternativamente, o instalador oficial da raylib já inclui o `w64devkit` com tudo configurado.

> **Nota:** O jogo não roda mais no terminal; ao executar, uma janela gráfica será aberta. No WSL, é necessário ter suporte a interface gráfica (WSLg, no Windows 11).

### Compilação e Execução

**Clonar o repositório:**
```bash
git clone [https://github.com/thelimacosta/pizzalign.git](https://github.com/thelimacosta/pizzalign.git)
cd pizzalign
```

**Compilar o projeto:**
Utilize o `Makefile` incluído na raiz do projeto:
```bash
make
```
> O `Makefile` já faz a ligação com a raylib (`-lraylib` e dependências do sistema). Se a raylib estiver instalada em um caminho fora do padrão, ajuste as variáveis `CFLAGS` e `LDFLAGS` no `Makefile`.

**Iniciar o jogo:**
Após a compilação, execute o binário gerado:
```bash
./pizzalign
```
> Os arquivos de recursos (imagens, fontes, sons), se houver, devem permanecer na pasta do projeto ao executar o jogo.

## Equipe

- 🖧 Matheus Pinheiro Lima Costa (mplc@cesar.school)
- 🖧 Nara de Melo Santos (nms2@cesar.school)
- 🖧 Pedro Correia Antunes (pca2@cesar.school)

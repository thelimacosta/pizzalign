<h1 align="center">🍕 PizzAlign</h1>

> Projeto desenvolvido para a disciplina de **Programação Imperativa e Funcional (PIF)**. Consiste em um jogo casual dinâmico para dois jogadores construído em **C**, com interface gráfica em janela utilizando a biblioteca **raylib**, aplicando conceitos de estruturas, ponteiros, matrizes, alocação dinâmica, listas encadeadas e manipulação de arquivos.

## Sobre

**PizzAlign** mistura a lógica de quebra-cabeças com o dinamismo dos jogos de queda de blocos. O design e as mecânicas foram inspirados em diversos títulos:

- **Tetris:** A gravidade contínua, o formato de tabuleiro em matriz e a necessidade de limpar o espaço antes que transborde.
- **Half Moon (Google):** A ideia central de juntar frações para formar um círculo completo (neste caso, uma pizza).
- **Suika Game (Watermelon Game):** A mecânica de colisão e fusão baseada em compatibilidade.
- **Overcooked:** O visual, com texturas de ingredientes, balcões de madeira e animações.
- **Make7 / 2048:** As cartas bem marcadas no tabuleiro, com efeitos de escala ao fundir peças.
- **Tetris Effect:** O feedback visual que torna cada combinação satisfatória, com os pontos subindo na tela ao fechar uma pizza inteira.

## Regras

- **Dois jogadores:** Os jogadores se revezam, uma carta cada. O Jogador 1 começa.
- **Gravidade e Queda:** Cartas de pizza caem do topo da tela em direção ao fundo do tabuleiro (10 x 20 quadradinhos; cada carta ocupa 2 x 2). São 8 cartas: caixa vazia, uma fatia (esquerda ou direita), metade (esquerda ou direita), faltando uma fatia (esquerda ou direita) e pizza inteira.
- **Sistema de Colisão e Fusão:**
  - Quando uma carta pousa, o jogo olha a carta de baixo e as dos lados (esquerda e direita), de qualquer jogador.
  - Se a vizinha **completa** a pizza com ela, as duas se fundem e somem. Só encaixa o lado equivalente: fatia esquerda com faltando-fatia esquerda, fatia direita com faltando-fatia direita, metade esquerda com metade direita, caixa vazia com pizza inteira.
  - Se a vizinha for **exatamente a mesma carta**, as duas também se fundem e somem.
  - Em qualquer outro caso, a fusão falha e a carta apenas se empilha.
- **Pontuação:** Completar uma pizza vale **3 pontos**; juntar duas cartas iguais vale **1 ponto**. Quando as cartas somem, as de cima descem e podem pontuar de novo (cascata).
- **Tomate:** O tomate com X nunca pontua nem se funde: só ocupa espaço. Ele aparece mais conforme a pontuação sobe.
- **Dificuldade:** Depois de 30 pontos, a queda fica um pouco mais rápida a cada ponto.
- **Fim de Jogo:** A partida termina se a pilha de cartas atingir o limite superior do tabuleiro. Vence quem tiver mais pontos.
- **Ranking:** O jogo salva o nome e os pontos do vencedor no arquivo `recordes.txt`.

## Como Jogar

Você controla a carta apenas enquanto ela está caindo, e só o jogador da vez joga. Utilize o teclado para gerenciar os movimentos:

| Tecla | Ação |
| :--- | :--- |
| `A` / `D` | Jogador 1: move a carta em queda para a esquerda / direita |
| `S` | Jogador 1: acelera a queda da carta (*soft drop*) |
| `Seta para Esquerda` / `Seta para Direita` | Jogador 2: move a carta em queda para a esquerda / direita |
| `Seta para Baixo` | Jogador 2: acelera a queda da carta (*soft drop*) |
| `P` | Pausa o jogo |
| `M` / `Menu` | Volta ao menu do jogo |
| `Q` | Sai do jogo |

## Execução

### Pré-requisitos

Para executar este jogo, você precisará de um ambiente baseado em Unix (Linux, macOS ou WSL no Windows) ou do Windows com o `w64devkit`, com as seguintes ferramentas instaladas:

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
git clone https://github.com/thelimacosta/pizzalign.git
cd pizzalign
```

**Compilar o projeto:**
Utilize o `Makefile` incluído na raiz do projeto:
```bash
make
```
> O `Makefile` já faz a ligação com a raylib (`-lraylib` e dependências do sistema). Se a raylib estiver em uma pasta fora do padrão, informe o caminho da pasta `src` dela, por exemplo `make RAYLIB=C:/pizzalign/raylib/src`.

Sem o `make`, o comando direto no Windows é:
```bash
gcc main.c motor.c -o pizzalign.exe -I caminho/raylib/src -L caminho/raylib/src -lraylib -lopengl32 -lgdi32 -lwinmm -lm
```

**Iniciar o jogo:**
Após a compilação, execute o binário gerado (`pizzalign.exe` no Windows):
```bash
./pizzalign
```
> As imagens e as fontes já vão embutidas no executável (arquivo `assets_embutidos.h`). Para trocar algum arquivo da pasta `assets/`, rode `python gerar_embutidos.py` e compile de novo.

## Créditos

- Fonte **Redaction 35** © MCKL Inc. (licença em `LICENCA-fonte-Redaction.txt`).
- Foto de fundo: Rito Margolis.

## Equipe

- 🖧 Matheus Pinheiro Lima Costa (mplc@cesar.school)
- 🖧 Nara de Melo Santos (nms2@cesar.school)
- 🖧 Pedro Correia Antunes (pca2@cesar.school)

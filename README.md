# 🍕 Pizzalign

**Pizzalign** é um jogo de estratégia com temática de pizzaria. Posicione cartas de pizza no tabuleiro, forme combinações e tente marcar mais pontos que seu adversário!

## Como jogar

O objetivo é conseguir a maior pontuação usando as cartas disponíveis para criar combinações no tabuleiro.

- Escolha o tamanho do tabuleiro: **3×3, 4×4 ou 5×5**.
- Jogue contra o chef ou em uma partida local para dois jogadores no mesmo dispositivo.
- Selecione uma carta da mão e coloque-a em uma casa disponível.
- As cartas permanecem no tabuleiro, permitindo que combinações sejam construídas ao longo da partida.

### Combinações e pontuação

O jogo recompensa diferentes padrões de cartas:

- **Cartas iguais adjacentes:** +1 ponto.
- **Cartas complementares:** +2 pontos.
- **Sequências de fatias em linha:** +3 pontos por sequência válida de três ou mais cartas.

A pontuação é calculada a partir das combinações formadas no tabuleiro.

## Tecnologias

- **HTML, CSS e JavaScript:** interface, apresentação do tabuleiro e interação.
- **C:** implementação do motor de regras e pontuação.
- **WebAssembly (WASM):** permite executar o código C no navegador.

## Como executar?

1. Baixe ou clone este repositório.
2. Abra o arquivo `Pizzalign_C_WASM.html` em um navegador moderno.
3. Se o projeto estiver usando o módulo WebAssembly como arquivo separado, mantenha `pizzalign_engine.wasm` na mesma pasta do HTML.
4. Comece uma partida pelo menu do jogo.

Não é necessário instalar um compilador C para jogar.

## Arquivos principais

| Arquivo | Descrição |
|---|---|
| `Pizzalign_C_WASM.html` | Página principal do jogo |
| `pizzalign_engine.c` | Código-fonte do motor em C |
| `pizzalign_engine.wasm` | Módulo WebAssembly usado pelo jogo |

## Colaboradores

Matheus Costa | mplc@cesar.school

Nara de Melo | mns2@cesar.school

Pedro Correia | pca2@cesar.school

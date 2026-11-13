/*
 * PIZZALIGN - main.c  (interface em raylib; as regras e a matematica ficam no motor.c)
 *
 * Compilar no Windows (MinGW), dentro da pasta do jogo:
 *   gcc main.c motor.c -o pizzalign.exe -I ../../raylib/src -L ../../raylib/src -lraylib -lopengl32 -lgdi32 -lwinmm -lm
 * Compilar no Linux:
 *   gcc main.c motor.c -o pizzalign -lraylib -lGL -lm -lpthread -ldl -lrt -lX11
 *
 * assets_embutidos.h guarda o fundo, a logo, as cartas e as fontes dentro do codigo
 * (para trocar algum arquivo de assets/, rode gerar_embutidos.py e compile de novo).
 *
 * Dois jogadores se revezam, uma carta cada vez:
 *   Jogador 1 (comeca): A / D = mover | S = descer mais rapido
 *   Jogador 2         : seta esquerda / direita = mover | seta para baixo = descer
 *   P = pausa | M = menu | Q = sair
 *
 * Regras (estao no motor.c): a carta pousa e empilha. Ao pousar, se a vizinha de baixo, da esquerda
 * ou da direita (de qualquer jogador) somar 6 com ela, vale 3 pontos; se for igual, vale 1 ponto.
 * Quando pontua, as duas cartas se fundem e somem; senao, so acumulam. Tomate nunca pontua.
 *
 * Mapa do arquivo: 1) recursos  2) texto  3) desenho  4) logica da partida  5) main
 */
#include "raylib.h"
#include "motor.h"
#include "assets_embutidos.h"
#include <math.h>
#include <ctype.h>
#include <string.h>
#include <time.h>

/* ---------- tela (medidas "logicas": a janela so escala tudo para caber no monitor) ---------- */
#define LARGURA   640
#define ALTURA    980                         /* mesma proporcao da foto de fundo (669 x 1024) */
#define QUAD      36                          /* lado de um quadradinho do tabuleiro */
#define CARTA     (QUAD * CASAS_CARTA)        /* cada carta ocupa 2x2 quadradinhos = 72 px */
#define MESA_L    (larguraTab * CARTA)        /* tabuleiro: 10 x 20 quadradinhos = 360 x 720 px */
#define MESA_A    (alturaTab * CARTA)
#define MESA_X    ((LARGURA - MESA_L) / 2)
#define MESA_Y    238

/* ---------- hierarquia dos textos (Redaction 35) ---------- */
#define T_TITULO  44    /* titulo de tela: Como jogar, Fim de jogo      (negrito) */
#define T_BOTAO   34    /* botoes do menu                               (negrito) */
#define T_SUB     28    /* subtitulos: Vez do Jogador, Ranking          (negrito) */
#define T_CORPO   22    /* texto corrido, pontos, ranking               (normal)  */
#define T_PEQUENO 17    /* dicas e legendas                             (normal)  */

#define ARQUIVO_RECORDES  "recordes.txt"
#define TAM_NOME_DIGITADO 12
#define VIDA_FANTASMA     0.55f
#define COR(r, g, b) ((Color){(r), (g), (b), 255})

static const Color TINTA     = {70, 38, 18, 255};          /* texto sobre o pergaminho */
static const Color COR_J[2]  = {{176, 92, 18, 255}, {30, 96, 160, 255}};      /* texto de cada jogador */
static const Color COR_VEZ[2] = {{255, 205, 70, 255}, {100, 190, 255, 255}};  /* faixa da coluna no tabuleiro */

typedef enum { TELA_MENU, TELA_COMO, TELA_JOGO } Tela;
typedef struct { float x, y, vida; int valor; } Fantasma;          /* carta que acabou de pontuar, sumindo */
typedef struct { int pontos; bool pizza; float x, y, vida; } Popup; /* "+3" / "+1" subindo */
typedef struct {
    float tempo, dispX, dispY, tEsq, tDir;
    Fantasma fantasmas[2 * MAX_PARES];
    Popup popups[MAX_PARES];
    char nome[TAM_NOME_DIGITADO + 1];
    int tamNome, vez;        /* vez: 0 = jogador 1, 1 = jogador 2 */
    int pontos[2];           /* pontos de cada jogador (o motor guarda so o total) */
    bool salvo, pausado;
} Partida;

/* ================================================================
 *  1. RECURSOS (tudo vem de assets_embutidos.h)
 * ================================================================ */
static Font fTitulo, fTexto;
static Texture2D spr_carta[9], spr_tomate, spr_play, spr_ajuda, spr_coluna, spr_fundo, spr_logo;   /* spr_carta[CARTA_CAIXA..CARTA_INTEIRA] */

static Texture2D textura(const unsigned char *dados, int tam) {
    Image img = LoadImageFromMemory(".png", dados, tam);
    if (img.data == NULL) { TraceLog(LOG_WARNING, "PIZZALIGN: uma imagem embutida nao carregou"); return (Texture2D){0}; }
    Texture2D t = LoadTextureFromImage(img);
    UnloadImage(img);
    SetTextureFilter(t, TEXTURE_FILTER_BILINEAR);
    return t;
}

static Font fonte(const unsigned char *dados, int tam) {
    int pontos[200], n = 0;                                  /* ASCII + letras acentuadas (Latin-1) */
    for (int c = 32; c < 127; c++) pontos[n++] = c;
    for (int c = 160; c < 256; c++) pontos[n++] = c;
    Font f = LoadFontFromMemory(".ttf", dados, tam, 64, pontos, n);
    if (f.texture.id == 0) { TraceLog(LOG_WARNING, "PIZZALIGN: a fonte nao carregou, usando a padrao"); return GetFontDefault(); }
    SetTextureFilter(f.texture, TEXTURE_FILTER_BILINEAR);
    return f;
}

/* colunas de marmore dos lados do tabuleiro (desenhadas em codigo) */
static Image gerar_coluna(int W, int H) {
    Image img = GenImageColor(W, H, BLANK);
    for (int y = 0; y < H; y++)
        for (int x = 0; x < W; x++) {
            int x0 = 6, x1 = W - 7;
            if (y < 8 || y >= H - 8)        { x0 = 0; x1 = W - 1; }          /* topo do capitel / base */
            else if (y < 16 || y >= H - 16) { x0 = 3; x1 = W - 4; }
            else if (y < 24 || y >= H - 24) { x0 = 5; x1 = W - 6; }
            if (x < x0 || x > x1) continue;
            float t = (float)(x - x0) / (float)(x1 - x0);
            int v = 246 - (int)(fabsf(t - 0.35f) * 90.0f);
            if (y >= 24 && y < H - 24 && ((x - x0) % 6) == 0) v -= 28;       /* caneluras */
            if ((y < 24 || y >= H - 24) && (y % 8) == 0) v -= 22;
            if (x == x0 || x == x1) v = 150;
            if ((x * 7 + y * 13) % 23 == 0) v -= 12;                         /* veios do marmore */
            ImageDrawPixel(&img, x, y, (Color){(unsigned char)v, (unsigned char)(v - 4), (unsigned char)(v - 12), 255});
        }
    return img;
}

static void carregar_recursos(void) {
    spr_carta[CARTA_CAIXA]    = textura(CARTA_CAIXA_PNG, CARTA_CAIXA_PNG_TAM);
    spr_carta[CARTA_FATIA_E]  = textura(CARTA_FATIA_E_PNG, CARTA_FATIA_E_PNG_TAM);
    spr_carta[CARTA_FATIA_D]  = textura(CARTA_FATIA_D_PNG, CARTA_FATIA_D_PNG_TAM);
    spr_carta[CARTA_METADE_E] = textura(CARTA_METADE_E_PNG, CARTA_METADE_E_PNG_TAM);
    spr_carta[CARTA_METADE_D] = textura(CARTA_METADE_D_PNG, CARTA_METADE_D_PNG_TAM);
    spr_carta[CARTA_FALTA_E]  = textura(CARTA_FALTA_E_PNG, CARTA_FALTA_E_PNG_TAM);
    spr_carta[CARTA_FALTA_D]  = textura(CARTA_FALTA_D_PNG, CARTA_FALTA_D_PNG_TAM);
    spr_carta[CARTA_INTEIRA]  = textura(CARTA_INTEIRA_PNG, CARTA_INTEIRA_PNG_TAM);
    spr_tomate = textura(TOMATE_PNG, TOMATE_PNG_TAM);
    spr_play   = textura(ICONE_PLAY_PNG, ICONE_PLAY_PNG_TAM);
    spr_ajuda  = textura(ICONE_AJUDA_PNG, ICONE_AJUDA_PNG_TAM);
    spr_fundo  = textura(FUNDO_PNG, FUNDO_PNG_TAM);
    spr_logo   = textura(LOGO_PNG, LOGO_PNG_TAM);
    Image col = gerar_coluna(40, MESA_A + 36);
    spr_coluna = LoadTextureFromImage(col);
    UnloadImage(col);
    fTitulo = fonte(FONTE_TITULO_TTF, FONTE_TITULO_TTF_TAM);
    fTexto  = fonte(FONTE_TEXTO_TTF, FONTE_TEXTO_TTF_TAM);
}

static void liberar_recursos(void) {
    for (int i = CARTA_CAIXA; i <= CARTA_INTEIRA; i++) UnloadTexture(spr_carta[i]);
    UnloadTexture(spr_tomate); UnloadTexture(spr_play); UnloadTexture(spr_ajuda); UnloadTexture(spr_coluna);
    UnloadTexture(spr_fundo);  UnloadTexture(spr_logo);
    if (fTitulo.texture.id != GetFontDefault().texture.id) UnloadFont(fTitulo);
    if (fTexto.texture.id != GetFontDefault().texture.id) UnloadFont(fTexto);
}

/* ================================================================
 *  2. TEXTO
 * ================================================================ */
static void escreve(Font f, const char *t, float x, float y, float tam, Color cor) {
    DrawTextEx(f, t, (Vector2){x, y}, tam, 1, cor);
}
static float mede(Font f, const char *t, float tam) { return MeasureTextEx(f, t, tam, 1).x; }
static void escreve_centro(Font f, const char *t, float cx, float y, float tam, Color cor) {
    escreve(f, t, cx - mede(f, t, tam) / 2, y, tam, cor);
}
/* texto direto sobre a foto de fundo: ganha sombra para ler bem */
static void sombra_centro(Font f, const char *t, float cx, float y, float tam, Color cor) {
    escreve_centro(f, t, cx + 2, y + 2, tam, Fade(BLACK, 0.7f));
    escreve_centro(f, t, cx, y, tam, cor);
}

/* escreve um paragrafo quebrando as linhas; devolve o y da proxima linha */
static float paragrafo(const char *txt, float x, float y, float larg, float tam, Color cor) {
    char linha[512] = "", palavra[64], teste[512];
    const char *p = txt;
    while (*p) {
        int n = 0;
        while (*p && *p != ' ' && n < 63) palavra[n++] = *p++;
        palavra[n] = '\0';
        while (*p == ' ') p++;
        snprintf(teste, sizeof teste, "%s%s%s", linha, linha[0] ? " " : "", palavra);
        if (mede(fTexto, teste, tam) > larg && linha[0]) {
            escreve(fTexto, linha, x, y, tam, cor);
            y += tam * 1.22f;
            snprintf(linha, sizeof linha, "%s", palavra);
        } else {
            snprintf(linha, sizeof linha, "%s", teste);
        }
    }
    if (linha[0]) { escreve(fTexto, linha, x, y, tam, cor); y += tam * 1.22f; }
    return y;
}

/* ================================================================
 *  3. DESENHO
 * ================================================================ */
static void desenhar_textura(Texture2D t, float x, float y, float tam, float alfa) {
    DrawTexturePro(t, (Rectangle){0, 0, (float)t.width, (float)t.height}, (Rectangle){x, y, tam, tam},
                   (Vector2){0, 0}, 0, Fade(WHITE, alfa));
}

/* carta: CARTA_CAIXA..CARTA_INTEIRA = os desenhos das pizzas, TOMATE = bloco */
static void desenhar_carta(int valor, float x, float y, float tam, float alfa) {
    Texture2D t = (valor >= TOMATE) ? spr_tomate : spr_carta[valor < CARTA_CAIXA ? CARTA_CAIXA : valor];
    if (t.id == 0) { DrawRectangle((int)x, (int)y, (int)tam, (int)tam, Fade(COR(214, 160, 44), alfa)); return; }
    desenhar_textura(t, x, y, tam, alfa);
}

/* a foto inteira, uma vez so, esticada para a janela vertical (mesma proporcao, sem espelhar) */
static void desenhar_fundo(void) {
    DrawTexturePro(spr_fundo, (Rectangle){0, 0, (float)spr_fundo.width, (float)spr_fundo.height},
                   (Rectangle){0, 0, LARGURA, ALTURA}, (Vector2){0, 0}, 0, WHITE);
}

/* logo pura (sem quadrado atras); so um contorno escuro fino para ela ler sobre o ceu */
#define LOGO_CONTORNO 1
static void desenhar_logo(float cx, float y, float w) {
    if (spr_logo.id == 0) { sombra_centro(fTitulo, "Pizzalign", cx, y, T_TITULO, COR(255, 214, 90)); return; }
    float h = w * spr_logo.height / spr_logo.width;
    Rectangle src = {0, 0, (float)spr_logo.width, (float)spr_logo.height};
#if LOGO_CONTORNO
    float o = fmaxf(2.0f, w / 150.0f);
    const float dx[8] = {-1, 0, 1, -1, 1, -1, 0, 1}, dy[8] = {-1, -1, -1, 0, 0, 1, 1, 1};
    for (int i = 0; i < 8; i++)
        DrawTexturePro(spr_logo, src, (Rectangle){cx - w / 2 + dx[i] * o, y + dy[i] * o, w, h}, (Vector2){0, 0}, 0, COR(12, 16, 28));
#endif
    DrawTexturePro(spr_logo, src, (Rectangle){cx - w / 2, y, w, h}, (Vector2){0, 0}, 0, WHITE);
}

/* pergaminho com rolos nas pontas (moldura do painel, como no rascunho) */
static void desenhar_pergaminho(Rectangle r) {
    Color papel = COR(238, 220, 178), borda = COR(120, 82, 40), rolo = COR(206, 172, 110), rolo2 = COR(150, 112, 60);
    DrawRectangle((int)r.x + 6, (int)r.y + 8, (int)r.width, (int)r.height, Fade(BLACK, 0.35f));
    DrawRectangleRec(r, papel);
    DrawRectangleLinesEx(r, 3, borda);
    for (int i = 0; i < 2; i++) {
        float y = (i == 0) ? r.y - 14 : r.y + r.height - 8;
        DrawRectangleRounded((Rectangle){r.x - 14, y, r.width + 28, 22}, 0.9f, 8, rolo);
        DrawRectangle((int)r.x - 4, (int)y + 15, (int)r.width + 8, 3, rolo2);
        for (int lado = 0; lado < 2; lado++) {
            Vector2 c = {(lado ? r.x + r.width + 14 : r.x - 14), y + 11};
            DrawCircleV(c, 12, rolo);
            DrawRing(c, 2, 3.5f, 0, 360, 16, rolo2);
            DrawRing(c, 6, 7.5f, 0, 360, 20, rolo2);
            DrawRing(c, 11, 12, 0, 360, 24, borda);
        }
    }
}

static int linha_de_pouso(const EstadoJogo *j) {
    int y = j->fatiaAtual.y;
    while (y + 1 < alturaTab && j->tab[y + 1][j->fatiaAtual.x] == VAZIA) y++;
    return y;
}

/* tabuleiro: xadrez de 10 x 20 quadradinhos; cada carta ocupa 2 x 2 deles */
static void desenhar_mesa(const EstadoJogo *jogo, float dispX, float dispY, int vez) {
    DrawRectangle(MESA_X - 5, MESA_Y - 5, MESA_L + 10, MESA_A + 10, COR(60, 30, 20));
    for (int r = 0; r < alturaTab * CASAS_CARTA; r++)
        for (int c = 0; c < larguraTab * CASAS_CARTA; c++)
            DrawRectangle(MESA_X + c * QUAD, MESA_Y + r * QUAD, QUAD, QUAD,
                          ((r + c) % 2 == 0) ? COR(160, 30, 38) : COR(240, 226, 198));

    for (int r = 0; r < alturaTab; r++)
        for (int c = 0; c < larguraTab; c++)
            if (jogo->tab[r][c] != VAZIA)
                desenhar_carta(jogo->tab[r][c], (float)(MESA_X + c * CARTA), (float)(MESA_Y + r * CARTA), CARTA, 1.0f);

    if (!jogo->gameOver) {
        int x = jogo->fatiaAtual.x, v = jogo->fatiaAtual.pedacos;
        DrawRectangle(MESA_X + x * CARTA, MESA_Y, CARTA, MESA_A, Fade(COR_VEZ[vez], 0.22f));              /* coluna de quem joga */
        desenhar_carta(v, (float)(MESA_X + x * CARTA), (float)(MESA_Y + linha_de_pouso(jogo) * CARTA), CARTA, 0.30f);
        desenhar_carta(v, MESA_X + dispX * CARTA, MESA_Y + dispY * CARTA, CARTA, 1.0f);                   /* carta caindo */
    }
}

static void desenhar_colunas(void) {
    Rectangle src = {0, 0, (float)spr_coluna.width, (float)spr_coluna.height};
    DrawTexturePro(spr_coluna, src, (Rectangle){MESA_X - 52, MESA_Y - 18, 40, MESA_A + 36}, (Vector2){0, 0}, 0, WHITE);
    DrawTexturePro(spr_coluna, src, (Rectangle){MESA_X + MESA_L + 12, MESA_Y - 18, 40, MESA_A + 36}, (Vector2){0, 0}, 0, WHITE);
}

static void desenhar_painel(const EstadoJogo *jogo, const Partida *pa, int nivel) {
    float x = 62, y = 112;
    desenhar_pergaminho((Rectangle){40, y, 560, 100});

    escreve(fTitulo, TextFormat("Vez do Jogador %d", pa->vez + 1), x, y + 6, T_SUB, COR_J[pa->vez]);
    escreve(fTexto, TextFormat("J1 Pontos: %d", pa->pontos[0]), x, y + 42, T_CORPO, COR_J[0]);
    escreve(fTexto, TextFormat("J2 Pontos: %d", pa->pontos[1]), x, y + 68, T_CORPO, COR_J[1]);

    escreve(fTexto, "Próximas", 318, y + 6, T_PEQUENO, TINTA);
    const NoFatia *n = jogo->filaFatias;
    for (int i = 0; i < 3 && n; i++, n = n->proximo) {
        Rectangle caixa = {318 + i * 52, y + 30, 46, 46};
        DrawRectangleRec(caixa, COR(52, 36, 30));
        DrawRectangleLinesEx(caixa, 2, COR(150, 112, 60));
        desenhar_carta(n->pedacos, caixa.x + 3, caixa.y + 3, 40, 1.0f);
    }

    escreve(fTexto, TextFormat("Nível %d", nivel), 494, y + 6, T_CORPO, TINTA);
    escreve(fTexto, "M : Menu", 494, y + 38, T_CORPO, TINTA);
    escreve(fTexto, "Q : Sair", 494, y + 66, T_CORPO, TINTA);
}

/* "+3" com a caixa de pizza (soma 6) ou "+1" (iguais), subindo */
static void desenhar_popup(const Popup *p) {
    float a = fminf(1.0f, p->vida), tam = 42;
    Color cor = Fade(p->pizza ? COR(255, 226, 90) : COR(170, 255, 170), a);
    const char *t = TextFormat("+%d", p->pontos);
    float larg = mede(fTitulo, t, tam) + (p->pizza ? 54 : 0);
    float x = p->x + (CARTA - larg) / 2, y = p->y + 10;
    if (p->pizza) { desenhar_textura(spr_carta[CARTA_INTEIRA], x, y, 46, a); x += 54; }
    for (int i = 0; i < 4; i++)                                                  /* contorno escuro */
        escreve(fTitulo, t, x + (i % 2 ? 2 : -2), y + (i < 2 ? 2 : -2), tam, Fade(BLACK, 0.8f * a));
    escreve(fTitulo, t, x, y, tam, cor);
}

static Rectangle botao_menu(int i) { return (Rectangle){150, 480.0f + i * 84, 340, 64}; }

static void desenhar_menu(int opcao) {
    desenhar_logo(LARGURA / 2.0f, 120, 480);
    const char *nomes[2] = {"Novo Jogo", "Como Jogar"};
    for (int i = 0; i < 2; i++) {
        Rectangle b = botao_menu(i);
        bool sel = (i == opcao);
        DrawRectangleRounded((Rectangle){b.x - 3, b.y - 3, b.width + 6, b.height + 6}, 0.4f, 8,
                             sel ? COR(255, 214, 90) : COR(120, 82, 40));
        DrawRectangleRounded(b, 0.4f, 8, sel ? COR(86, 40, 26) : COR(46, 36, 30));
        Color cor = sel ? COR(255, 226, 120) : COR(236, 220, 190);
        float cy = b.y + b.height / 2;
        desenhar_textura(i == 0 ? spr_play : spr_ajuda, b.x + 20, cy - 24, 48, 1.0f);   /* icones dos botoes */
        escreve(fTitulo, nomes[i], b.x + 86, cy - 21, T_BOTAO, cor);
    }
    sombra_centro(fTexto, "W / S ou setas: escolher     Enter: confirmar     Q: sair", LARGURA / 2.0f, 920, T_PEQUENO, COR(255, 240, 210));
}

static void desenhar_como_jogar(void) {
    DrawRectangle(0, 0, LARGURA, ALTURA, Fade(BLACK, 0.25f));
    Rectangle p = {40, 76, 560, 840};
    desenhar_pergaminho(p);
    escreve_centro(fTitulo, "Como jogar", LARGURA / 2.0f, p.y + 14, T_TITULO, TINTA);

    float x = p.x + 36, l = p.width - 72, y = p.y + 82;
    y = paragrafo("Jogador 1: A e D movem, S desce. Jogador 2: setas esquerda e direita movem, seta para baixo desce. "
                  "Cada um joga uma carta e passa a vez.", x, y, l, T_CORPO, TINTA);
    y = paragrafo("A carta pousa e empilha. Pontua se a de baixo, da esquerda ou da direita (de qualquer jogador) "
                  "a completar (3 pontos) ou for igual (1 ponto). Só encaixa o lado equivalente. "
                  "Ao pontuar, as duas somem e as de cima descem.", x, y + 8, l, T_CORPO, TINTA);
    y = paragrafo("Depois de 30 pontos a queda acelera um pouco. Quando o topo enche, vence quem tiver mais pontos.",
                  x, y + 8, l, T_CORPO, TINTA);

    float ry = y + 8, ts = 46;                               /* exemplos */
    desenhar_carta(CARTA_METADE_E, x, ry, ts, 1);             escreve(fTitulo, "+", x + 58, ry + 8, 34, TINTA);
    desenhar_carta(CARTA_METADE_D, x + 90, ry, ts, 1);        escreve(fTitulo, "=", x + 148, ry + 8, 34, TINTA);
    desenhar_carta(CARTA_INTEIRA, x + 190, ry, ts, 1);        escreve(fTitulo, "+3 pontos", x + 256, ry + 8, T_SUB, TINTA);
    ry += ts + 8;
    desenhar_carta(CARTA_FATIA_E, x, ry, ts, 1);              escreve(fTitulo, "+", x + 58, ry + 8, 34, TINTA);
    desenhar_carta(CARTA_FALTA_E, x + 90, ry, ts, 1);         escreve(fTitulo, "=", x + 148, ry + 8, 34, TINTA);
    desenhar_carta(CARTA_INTEIRA, x + 190, ry, ts, 1);        escreve(fTitulo, "+3 pontos", x + 256, ry + 8, T_SUB, TINTA);
    ry += ts + 8;
    desenhar_carta(CARTA_FATIA_E, x, ry, ts, 1);              escreve(fTitulo, "+", x + 58, ry + 8, 34, TINTA);
    desenhar_carta(CARTA_FATIA_E, x + 90, ry, ts, 1);         escreve(fTitulo, "iguais", x + 148, ry + 12, T_CORPO, TINTA);
    escreve(fTitulo, "+1 ponto", x + 256, ry + 8, T_SUB, TINTA);
    ry += ts + 8;
    desenhar_carta(CARTA_INTEIRA, x, ry, ts, 1);              escreve(fTitulo, "+", x + 58, ry + 8, 34, TINTA);
    desenhar_carta(CARTA_CAIXA, x + 90, ry, ts, 1);           escreve(fTexto, "completa", x + 148, ry + 12, T_CORPO, TINTA);
    escreve(fTitulo, "+3 pontos", x + 256, ry + 8, T_SUB, TINTA);
    ry += ts + 10;
    desenhar_carta(TOMATE, x, ry, ts, 1);
    escreve(fTexto, "Tomate: não funde com nada, só ocupa espaço.", x + 66, ry + 2, T_CORPO, TINTA);
    escreve(fTexto, "Aparece mais conforme os pontos sobem.", x + 66, ry + 26, T_CORPO, TINTA);

    escreve_centro(fTexto, "Enter ou M: voltar ao menu", LARGURA / 2.0f, p.y + p.height - 36, T_PEQUENO, TINTA);
}

static int vencedor(const Partida *pa) {
    if (pa->pontos[0] == pa->pontos[1]) return -1;
    return (pa->pontos[0] > pa->pontos[1]) ? 0 : 1;
}

static void desenhar_fim(const Recorde *recordes, const Partida *pa) {
    int v = vencedor(pa);
    DrawRectangle(0, 0, LARGURA, ALTURA, Fade(BLACK, 0.55f));
    Rectangle p = {70, 220, 500, 540};
    desenhar_pergaminho(p);
    escreve_centro(fTitulo, "Fim de jogo", LARGURA / 2.0f, p.y + 14, T_TITULO, TINTA);
    escreve_centro(fTexto, TextFormat("Jogador 1: %d      Jogador 2: %d", pa->pontos[0], pa->pontos[1]),
                   LARGURA / 2.0f, p.y + 76, T_CORPO, TINTA);
    escreve_centro(fTitulo, v < 0 ? "Empate!" : TextFormat("Venceu o Jogador %d!", v + 1),
                   LARGURA / 2.0f, p.y + 108, T_SUB, v < 0 ? TINTA : COR_J[v]);

    if (!pa->salvo) {
        escreve(fTexto, v < 0 ? "Digite um nome:" : "Nome do vencedor:", p.x + 60, p.y + 184, T_CORPO, TINTA);
        DrawRectangle((int)p.x + 60, (int)p.y + 218, 380, 50, COR(52, 36, 30));
        escreve(fTitulo, pa->nome, p.x + 72, p.y + 224, T_SUB, COR(255, 226, 120));
        if (((int)(GetTime() * 2)) % 2 == 0) escreve(fTitulo, "_", p.x + 72 + mede(fTitulo, pa->nome, T_SUB), p.y + 224, T_SUB, WHITE);
        escreve(fTexto, "Enter para salvar", p.x + 60, p.y + 284, T_PEQUENO, TINTA);
    } else {
        escreve(fTitulo, "Ranking", p.x + 60, p.y + 172, T_SUB, TINTA);
        int i = 0;
        for (const Recorde *r = recordes; r && i < 5; r = r->prox, i++) {
            escreve(fTexto, TextFormat("%d. %s", i + 1, r->nome), p.x + 60, p.y + 214 + i * 32, T_CORPO, TINTA);
            const char *pts = TextFormat("%d", r->pontos);
            escreve(fTexto, pts, p.x + p.width - 60 - mede(fTexto, pts, T_CORPO), p.y + 214 + i * 32, T_CORPO, TINTA);
        }
        escreve_centro(fTexto, "R: jogar de novo     M: menu     Q: sair", LARGURA / 2.0f, p.y + p.height - 44, T_PEQUENO, TINTA);
    }
}

/* ================================================================
 *  4. LOGICA DA PARTIDA (chama o motor)
 * ================================================================ */
static void nova_partida(Partida *p, EstadoJogo *jogo) {
    iniciarJogo(jogo);
    memset(p, 0, sizeof *p);
    p->dispX = (float)jogo->fatiaAtual.x;
}

/* ate 30 pontos a queda e fixa (0,55 s por linha); depois fica um pouquinho mais rapida a cada ponto */
static float intervalo_de_queda(int pontos) {
    float t = 0.55f;
    if (pontos > 30) t -= 0.004f * (pontos - 30);
    return t < 0.18f ? 0.18f : t;
}

/* tomates: o motor so cria cartas de 1 a 5, entao trocamos a ultima da fila (a recem-criada) por um tomate.
 * Eles so aparecem depois de 10 pontos e vao ficando mais comuns (no maximo 20%). */
static void sortear_tomate(EstadoJogo *jogo) {
    int chance = (jogo->pontuacao < 10) ? 0 : 2 + 2 * (jogo->pontuacao / 10);
    if (chance > 20) chance = 20;
    if (rand() % 100 >= chance) return;
    NoFatia *n = jogo->filaFatias;
    while (n && n->proximo) n = n->proximo;
    if (n) n->pedacos = TOMATE;
}

/* mostra os efeitos dos pares que pontuaram na ultima carta que pousou */
static void registrar_efeitos(Partida *pa, const EstadoJogo *jogo) {
    for (int i = 0; i < jogo->numPares; i++) {
        const Par *p = &jogo->pares[i];
        float x1 = (float)(MESA_X + p->col1 * CARTA), y1 = (float)(MESA_Y + p->lin1 * CARTA);
        float x2 = (float)(MESA_X + p->col2 * CARTA), y2 = (float)(MESA_Y + p->lin2 * CARTA);
        for (int k = 0; k < 2 * MAX_PARES; k++)
            if (pa->fantasmas[k].vida <= 0) { pa->fantasmas[k] = (Fantasma){x1, y1, VIDA_FANTASMA, p->valor1}; break; }
        for (int k = 0; k < 2 * MAX_PARES; k++)
            if (pa->fantasmas[k].vida <= 0) { pa->fantasmas[k] = (Fantasma){x2, y2, VIDA_FANTASMA, p->valor2}; break; }
        for (int k = 0; k < MAX_PARES; k++)
            if (pa->popups[k].vida <= 0) {
                pa->popups[k] = (Popup){p->pontos, p->pontos == PONTOS_PIZZA, (x1 + x2) / 2, (y1 + y2) / 2, 1.4f};
                break;
            }
    }
}

/* uma descida. Quando a carta pousa: o motor pontua, o ponto vai para quem jogou e passa a vez */
static void avancar(Partida *pa, EstadoJogo *jogo) {
    int antes = jogo->pecasFixadas;
    descer_peca(jogo);
    if (jogo->pecasFixadas == antes) return;                 /* so desceu uma linha */

    pa->pontos[pa->vez] += jogo->ganhoUltima;
    registrar_efeitos(pa, jogo);
    sortear_tomate(jogo);
    pa->vez = 1 - pa->vez;
    pa->tEsq = pa->tDir = 0;
    pa->dispX = (float)jogo->fatiaAtual.x;                   /* carta nova nao desliza */
}

static bool tecla_com_repeticao(int tecla, float *t, float dt) {
    if (IsKeyPressed(tecla)) { *t = 0; return true; }
    if (!IsKeyDown(tecla)) { *t = 0; return false; }
    *t += dt;
    if (*t >= 0.22f) { *t -= 0.06f; return true; }
    return false;
}

static void atualizar_partida(Partida *pa, EstadoJogo *jogo, float dt, float intervalo) {
    if (IsKeyPressed(KEY_P)) pa->pausado = !pa->pausado;
    if (pa->pausado) return;

    /* so as teclas de quem esta na vez valem: J1 = A D S, J2 = as tres setas */
    int kEsq = pa->vez ? KEY_LEFT : KEY_A, kDir = pa->vez ? KEY_RIGHT : KEY_D, kBaixo = pa->vez ? KEY_DOWN : KEY_S;
    if (tecla_com_repeticao(kEsq, &pa->tEsq, dt)) mover_peca(jogo, -1, 0);
    if (tecla_com_repeticao(kDir, &pa->tDir, dt)) mover_peca(jogo, 1, 0);

    pa->tempo += dt;
    float passo = IsKeyDown(kBaixo) ? fminf(intervalo, 0.05f) : intervalo;
    if (pa->tempo >= passo) { pa->tempo = 0; avancar(pa, jogo); }
}

static void atualizar_nome(Partida *pa, Recorde **recordes) {
    int ch;
    while ((ch = GetCharPressed()) > 0)
        if (ch < 128 && isalnum(ch) && pa->tamNome < TAM_NOME_DIGITADO) {
            pa->nome[pa->tamNome++] = (char)toupper(ch);
            pa->nome[pa->tamNome] = '\0';
        }
    if (IsKeyPressed(KEY_BACKSPACE) && pa->tamNome > 0) pa->nome[--pa->tamNome] = '\0';
    if (IsKeyPressed(KEY_ENTER)) {
        int v = vencedor(pa), pts = (v < 1) ? pa->pontos[0] : pa->pontos[1];   /* empate: os dois valem o mesmo */
        recorde_inserir(recordes, pa->tamNome ? pa->nome : "JOGADOR", pts);
        recordes_salvar(*recordes, ARQUIVO_RECORDES);
        pa->salvo = true;
    }
}

/* ================================================================
 *  5. MAIN
 * ================================================================ */
/* a janela e vertical e pode ter qualquer tamanho: o jogo escala tudo mantendo a proporcao */
static Camera2D camera_ajustada(void) {
    float z = fminf((float)GetScreenWidth() / LARGURA, (float)GetScreenHeight() / ALTURA);
    Camera2D c = {0};
    c.zoom = z;
    c.offset = (Vector2){(GetScreenWidth() - LARGURA * z) / 2, (GetScreenHeight() - ALTURA * z) / 2};
    return c;
}

int main(void) {
    srand((unsigned)time(NULL));
    SetConfigFlags(FLAG_VSYNC_HINT | FLAG_WINDOW_RESIZABLE);
    InitWindow(LARGURA / 2, ALTURA / 2, "Pizzalign");
    SetExitKey(KEY_NULL);                          /* so o Q (ou fechar a janela) encerra */
    SetTargetFPS(60);
    ChangeDirectory(GetApplicationDirectory());    /* recordes.txt fica ao lado do executavel */

    int mon = GetCurrentMonitor();                 /* janela vertical que cabe na tela */
    float z = (GetMonitorHeight(mon) > 0) ? GetMonitorHeight(mon) * 0.88f / ALTURA : 0.75f;
    if (z > 1.4f) z = 1.4f;
    if (z < 0.5f) z = 0.5f;
    SetWindowSize((int)(LARGURA * z), (int)(ALTURA * z));
    Vector2 mp = GetMonitorPosition(mon);
    SetWindowPosition((int)(mp.x + (GetMonitorWidth(mon) - LARGURA * z) / 2), (int)(mp.y + (GetMonitorHeight(mon) - ALTURA * z) / 2 - 20));
    SetWindowMinSize(LARGURA / 2, ALTURA / 2);

    carregar_recursos();

    EstadoJogo *jogo = estado_criar();
    if (!jogo) return 1;
    Partida pa;
    nova_partida(&pa, jogo);
    Recorde *recordes = recordes_carregar(ARQUIVO_RECORDES);

    Tela tela = TELA_MENU;
    int opcao = 0;
    bool sair = false;

    while (!sair && !WindowShouldClose()) {
        float dt = GetFrameTime();
        Camera2D cam = camera_ajustada();
        bool digitando = (tela == TELA_JOGO && jogo->gameOver && !pa.salvo);
        int nivel = jogo->pontuacao / 10 + 1;
        float intervalo = intervalo_de_queda(jogo->pontuacao);

        if (IsKeyPressed(KEY_Q) && !digitando) sair = true;

        /* ----- atualizar ----- */
        if (tela == TELA_MENU) {
            if (IsKeyPressed(KEY_DOWN) || IsKeyPressed(KEY_S) || IsKeyPressed(KEY_UP) || IsKeyPressed(KEY_W)) opcao = 1 - opcao;
            Vector2 m = GetScreenToWorld2D(GetMousePosition(), cam), mv = GetMouseDelta();
            bool clicou = false;
            for (int i = 0; i < 2; i++)
                if (CheckCollisionPointRec(m, botao_menu(i))) {
                    if (mv.x != 0 || mv.y != 0) opcao = i;
                    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) { opcao = i; clicou = true; }
                }
            if (clicou || IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_SPACE)) {
                if (opcao == 0) { nova_partida(&pa, jogo); tela = TELA_JOGO; }
                else tela = TELA_COMO;
            }
        } else if (tela == TELA_COMO) {
            if (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_M) || IsKeyPressed(KEY_ESCAPE) || IsKeyPressed(KEY_SPACE)) tela = TELA_MENU;
        } else {
            if (IsKeyPressed(KEY_M) && !digitando) tela = TELA_MENU;
            else if (!jogo->gameOver) atualizar_partida(&pa, jogo, dt, intervalo);
            else if (digitando) atualizar_nome(&pa, &recordes);
            else if (IsKeyPressed(KEY_R)) nova_partida(&pa, jogo);

            /* animacao suave da carta (so visual; a posicao real e a do motor) */
            pa.dispX += ((float)jogo->fatiaAtual.x - pa.dispX) * fminf(1.0f, 20.0f * dt);
            float frac = (!pa.pausado && !jogo->gameOver && pode_mover(jogo, 0, 1)) ? fminf(1.0f, pa.tempo / intervalo) : 0;
            pa.dispY = jogo->fatiaAtual.y + frac;
            for (int i = 0; i < 2 * MAX_PARES; i++) if (pa.fantasmas[i].vida > 0) pa.fantasmas[i].vida -= dt;
            for (int i = 0; i < MAX_PARES; i++)
                if (pa.popups[i].vida > 0) { pa.popups[i].vida -= dt; pa.popups[i].y -= 40 * dt; }
        }

        /* ----- desenhar ----- */
        BeginDrawing();
        ClearBackground(BLACK);
        BeginMode2D(cam);
        desenhar_fundo();
        if (tela == TELA_MENU) {
            desenhar_menu(opcao);
        } else if (tela == TELA_COMO) {
            desenhar_como_jogar();
        } else {
            desenhar_mesa(jogo, pa.dispX, pa.dispY, pa.vez);
            desenhar_colunas();
            desenhar_logo(LARGURA / 2.0f, 12, 330);
            desenhar_painel(jogo, &pa, nivel);

            for (int i = 0; i < 2 * MAX_PARES; i++)                  /* cartas que pontuaram: crescem e somem */
                if (pa.fantasmas[i].vida > 0) {
                    float t = pa.fantasmas[i].vida / VIDA_FANTASMA, tam = CARTA * (1 + (1 - t) * 0.35f);
                    desenhar_carta(pa.fantasmas[i].valor, pa.fantasmas[i].x - (tam - CARTA) / 2, pa.fantasmas[i].y - (tam - CARTA) / 2, tam, t);
                }
            for (int i = 0; i < MAX_PARES; i++) if (pa.popups[i].vida > 0) desenhar_popup(&pa.popups[i]);

            if (pa.pausado && !jogo->gameOver) {
                DrawRectangle(0, 0, LARGURA, ALTURA, Fade(BLACK, 0.55f));
                sombra_centro(fTitulo, "Pausado", LARGURA / 2.0f, 420, 64, WHITE);
            }
            if (jogo->gameOver) desenhar_fim(recordes, &pa);
        }
        EndMode2D();
        EndDrawing();
    }

    recordes_liberar(recordes);
    estado_liberar(jogo);
    liberar_recursos();
    CloseWindow();
    return 0;
}

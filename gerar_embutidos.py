#!/usr/bin/env python3
"""
gerar_embutidos.py - transforma as imagens e fontes da pasta assets/ no arquivo assets_embutidos.h,
que o main.c usa para o jogo abrir com tudo dentro do proprio executavel (sem depender de pastas).

Quando trocar qualquer arquivo de assets/ (logo, cartas, fundo, fonte), rode:
    python gerar_embutidos.py
e compile o jogo de novo.

Arquivos esperados em assets/:
    fundo.png        foto de fundo (uma imagem so, sem espelhar)
    logo.png         logo com fundo transparente
    carta_caixa.png, carta_fatia_esq/dir.png, carta_metade_esq/dir.png,
    carta_falta_esq/dir.png, carta_inteira.png     as 8 cartas de pizza
    tomate.png       bloco tomate com X
    icone_play.png, icone_ajuda.png   icones dos botoes do menu
    fonte_titulo.ttf Redaction 35 Bold    fonte_texto.ttf Redaction 35 Regular
"""
import os

AQUI = os.path.dirname(os.path.abspath(__file__))
PASTA = os.path.join(AQUI, "assets")

ITENS = [
    ("FUNDO_PNG", "fundo.png"), ("LOGO_PNG", "logo.png"),
    ("FONTE_TITULO_TTF", "fonte_titulo.ttf"), ("FONTE_TEXTO_TTF", "fonte_texto.ttf"),
    ("CARTA_CAIXA_PNG", "carta_caixa.png"), ("CARTA_FATIA_E_PNG", "carta_fatia_esq.png"),
    ("CARTA_FATIA_D_PNG", "carta_fatia_dir.png"), ("CARTA_METADE_E_PNG", "carta_metade_esq.png"),
    ("CARTA_METADE_D_PNG", "carta_metade_dir.png"), ("CARTA_FALTA_E_PNG", "carta_falta_esq.png"),
    ("CARTA_FALTA_D_PNG", "carta_falta_dir.png"), ("CARTA_INTEIRA_PNG", "carta_inteira.png"),
    ("TOMATE_PNG", "tomate.png"), ("ICONE_PLAY_PNG", "icone_play.png"), ("ICONE_AJUDA_PNG", "icone_ajuda.png"),
]

saida = ["/* assets_embutidos.h - GERADO por gerar_embutidos.py; nao edite a mao. */",
         "#ifndef ASSETS_EMBUTIDOS_H", "#define ASSETS_EMBUTIDOS_H", ""]
for nome, arquivo in ITENS:
    dados = open(os.path.join(PASTA, arquivo), "rb").read()
    saida.append(f"/* {arquivo} ({len(dados)} bytes) */")
    saida.append(f"static const unsigned char {nome}[] = {{")
    for i in range(0, len(dados), 24):
        saida.append(",".join(str(b) for b in dados[i:i + 24]) + ",")
    saida.append("};")
    saida.append(f"static const int {nome}_TAM = {len(dados)};")
    saida.append("")
saida.append("#endif")
open(os.path.join(AQUI, "assets_embutidos.h"), "w").write("\n".join(saida) + "\n")
print("assets_embutidos.h gerado com", len(ITENS), "arquivos")

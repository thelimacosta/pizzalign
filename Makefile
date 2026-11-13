# Pizzalign - compilar com:  make
# Se a raylib estiver em outra pasta:  make RAYLIB=C:/pizzalign/raylib/src
RAYLIB ?= raylib/src
CC = gcc

ifeq ($(OS),Windows_NT)
  EXE  = pizzalign.exe
  LIBS = -lraylib -lopengl32 -lgdi32 -lwinmm -lm
else
  EXE  = pizzalign
  LIBS = -lraylib -lGL -lm -lpthread -ldl -lrt -lX11
endif

all: $(EXE)

$(EXE): main.c motor.c motor.h assets_embutidos.h
	$(CC) main.c motor.c -o $(EXE) -I $(RAYLIB) -L $(RAYLIB) $(LIBS)

# so se voce trocar algo em assets/ (precisa de Python)
embutidos:
	python gerar_embutidos.py

clean:
	-rm -f pizzalign pizzalign.exe

.PHONY: all embutidos clean

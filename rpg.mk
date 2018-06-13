rpg.SRCS = rpg.c rpg-sdl.c gl3w.c
rpg.PKGS = sdl2 glu
rpg.CFLAGS = -Iinclude
rpg.LDFLAGS = -lm -ldl
TARGETS += rpg

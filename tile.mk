ifeq ($(OS),Windows_NT)
# Windows-only taget
tile.SRCS = tile-gdi.c tile.res
TARGETS += tile
else
tile.SRCS = tile-sdl.c tile.c game.c
tile.PKGS = sdl2
TARGETS += tile
endif

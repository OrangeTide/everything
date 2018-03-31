ifeq ($(OS),Windows_NT)
# Windows-only taget
tile.SRCS = tile.c tile.res
TARGETS += tile
else
tile.SRCS = tile-sdl.c
tile.PKGS = sdl2
TARGETS += tile
endif

tile.SRCS = tile-sdl.c tile.c game.c
tile.PKGS = sdl2
tile.CFLAGS = -pthread
ifeq ($(HAS_SDL),1)
TARGETS += tile
endif

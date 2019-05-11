tile.SRCS = tile-sdl.c tile.c game.c
tile.PKGS = sdl2
ifeq ($(HAS_SDL),1)
TARGETS += tile
endif

ansiview.SRCS = tile-sdl.c tile.c ansiview.c
ansiview.PKGS = sdl2
ifeq ($(HAS_SDL),1)
TARGETS += ansiview
endif

font2c.SRCS = font2c.c
TARGETS += font2c

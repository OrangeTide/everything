tile.SRCS = tile-sdl.c tile.c game.c
tile.PKGS = sdl2
TARGETS += tile

ansiview.SRCS = tile-sdl.c tile.c ansiview.c
ansiview.PKGS = sdl2
TARGETS += ansiview

font2c.SRCS = font2c.c
TARGETS += font2c

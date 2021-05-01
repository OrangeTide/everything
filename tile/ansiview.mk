ansiview.SRCS = tile-sdl.c tile.c ansiview.c
ansiview.PKGS = sdl2
ifeq ($(HAS_SDL),1)
TARGETS += ansiview
endif

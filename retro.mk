retro.SRCS = retro.c
retro.PKGS = sdl2 gl
ifeq ($(HAS_SDL),1)
TARGETS += retro
endif

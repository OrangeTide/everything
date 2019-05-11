bitscope.SRCS = bitscope.c ihex.c exebuf.c console.c bitscope-sdl.c
bitscope.PKGS = sdl2
ifeq ($(HAS_SDL),1)
TARGETS += bitscope
endif

# generates tables for our emulator
z80tab.SRCS = z80tab.c
ifeq ($(HAS_SDL),1)
TARGETS += z80tab
endif

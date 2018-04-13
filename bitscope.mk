bitscope.SRCS = bitscope.c ihex.c exebuf.c console.c bitscope-sdl.c
bitscope.PKGS = sdl2
TARGETS += bitscope

# generates tables for our emulator
z80tab.SRCS = z80tab.c
TARGETS += z80tab

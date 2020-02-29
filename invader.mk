invader.SRCS = invader.c
ifeq ($(OS),Windows_NT)
  invader.PKGS = opengl32 glu32
else
  invader.PKGS = gl glu x11
endif
TARGETS += invader

invader.SRCS = invader.c
ifeq ($(OS),Windows_NT)
  invader.PKGS = opengl32 glu32
  invader.SRCS += todo.c
else
  invader.PKGS = gl glu x11
  invader.SRCS += invader-xbase.c invader-glx.c
endif
TARGETS += invader

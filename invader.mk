invader.SRCS = invader.c
ifeq ($(OS),Windows_NT)
  invader.PKGS = glu
  invader.SRCS += invader-win32.c invader-wgl.c
else
  invader.PKGS = glu x11
  invader.SRCS += invader-xbase.c invader-glx.c
endif
TARGETS += invader

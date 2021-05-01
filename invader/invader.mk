invader.SRCS = invader.c
invader.CFLAGS = -Wall -W -Werror -std=gnu99 -flto
ifeq ($(OS),Windows_NT)
  invader.PKGS = glu
  invader.SRCS += invader-win32.c invader-wgl.c
else
  invader.PKGS = glu x11
  invader.SRCS += invader-xbase.c invader-glx.c
  invader.CFLAGS += -D_GNU_SOURCE=1
endif
TARGETS += invader

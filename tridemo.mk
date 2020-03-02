tridemo.SRCS = tridemo.c
ifeq ($(HAS_SDL),1)
TARGETS += tridemo
endif

ifeq ($(OS),Windows_NT)
  # Windows-only taget
  tridemo.PKGS = glu
else ifeq ($(OS),Linux)
  # Linux-only target
  tridemo.PKGS = glu x11
endif

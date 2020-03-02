spin.SRCS = spin.c
ifeq ($(HAS_SDL),1)
TARGETS += spin
endif

ifeq ($(OS),Windows_NT)
  # Windows-only taget
  spin.PKGS = glu
else ifeq ($(OS),Linux)
  # Linux-only target
  spin.PKGS = glu x11 m
endif

spin.SRCS = spin.c
ifeq ($(HAS_SDL),1)
TARGETS += spin
endif

ifeq ($(OS),Windows_NT)
  # Windows-only taget
  spin.PKGS = opengl32 glu32
else ifeq ($(OS),Linux)
  # Linux-only target
  spin.PKGS = gl glu x11 m
endif

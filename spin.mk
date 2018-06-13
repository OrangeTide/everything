spin.SRCS = spin.c
TARGETS += spin

ifeq ($(OS),Windows_NT)
  # Windows-only taget
  spin.PKGS = opengl32 glu
else ifeq ($(OS),Linux)
  # Linux-only target
  spin.PKGS = gl glu x11 m
endif

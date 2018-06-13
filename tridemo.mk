tridemo.SRCS = tridemo.c
TARGETS += tridemo

ifeq ($(OS),Windows_NT)
  # Windows-only taget
  tridemo.PKGS = opengl32 glu
else ifeq ($(OS),Linux)
  # Linux-only target
  tridemo.PKGS = gl glu x11
endif

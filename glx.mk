# Linux-only target
ifeq ($(OS),Linux)
glx.PKGS = gl glu x11
# glx.SRCS = glx.c
TARGETS += glx
endif

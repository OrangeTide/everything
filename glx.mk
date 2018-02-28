# Linux-only target
ifeq ($(OS),Linux)
glx.PKGS = gl glu x11 xrender
# glx.SRCS = glx.c
TARGETS += glx
endif

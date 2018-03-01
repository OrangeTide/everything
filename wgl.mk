ifeq ($(OS),Windows_NT)
# Windows-only target
wgl.PKGS = opengl32 glu32
# wgl.SRCS = wgl.c
TARGETS += wgl
endif

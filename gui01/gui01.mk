gui01.CPPFLAGS = -Inotifier
gui01.SRCS = gui01.c
ifeq ($(OS),Windows_NT)
	gui01.PKGS = gdi32
	gui01.SRCS += drv_gdi.c
else
	gui01.PKGS = x11
	gui01.SRCS += drv_xlib.c
endif
ifeq ($(HAS_SDL),1)
TARGETS += gui01
endif

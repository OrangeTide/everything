## OS detection
ifeq ($(HOSTOS),)
	HOSTOS := $(shell uname -s)
endif
ifeq ($(OS),)
	OS := $(HOSTOS)
endif

## Platform specific options
ifeq ($(OS),Windows_NT)
	X = .exe
	CFLAGS += -mwindows
	pkg-config-cflags = $(foreach pkg,$(1),$$(PKGCFLAGS.$(pkg)))
	pkg-config-libs = $(foreach pkg,$(1),$$(PKGLIBS.$(pkg)))
	CFLAGS = -mwindows -Wall -W -Os -ggdb
ifeq ($(HOSTOS),Linux)
	# based on my own cross-compile setup
	WINDRES = x86_64-w64-mingw32-windres
	CC = x86_64-w64-mingw32-gcc
	STRIP = x86_64-w64-mingw32-strip
else
	WINDRES = windres
	CC = gcc
	STRIP = strip
endif
# detect cmd.exe vs bash
ifeq ($(SHELL),)
	RM = del
else
	RM = rm -f
endif
else ifeq ($(OS),Linux)
	# first look up packages through PKGCFLAGS/PKGLIBS.
	# if not found, then fall back to pkg-config.
	pkg-config-cflags = $(foreach pkg,$(1),$$(if $$(PKGCFLAGS.$(pkg))$$(PKGLIBS.$(pkg)),$$(PKGCFLAGS.$(pkg)),$$(shell pkg-config --cflags $(pkg))))
	pkg-config-libs = $(foreach pkg,$(1),$$(if $$(PKGCFLAGS.$(pkg))$$(PKGLIBS.$(pkg)),$$(PKGLIBS.$(pkg)),$$(shell pkg-config --libs $(pkg))))
	CC = gcc
	STRIP = strip
	CFLAGS = -Wall -W -Os -ggdb
#TODO: include packages from *.mkpkg files
else
	$(error Unsupported build environment)
endif

#TODO: include packages/libraries from *.mkpkg files.

## Windows packages
ifeq ($(OS),Windows_NT)
	# opengl32
	PKGCFLAGS.opengl32 =
	PKGLIBS.opengl32 = -lopengl32
	# glu
	PKGCFLAGS.glu32 =
	PKGLIBS.glu32 = -lglu32
	# gl - an alias for what we typically consider OpenGL
	PKGCFLAGS.gl = $(PKGCFLAGS.opengl32) $(PKGCFLAGS.glu32)
	PKGLIBS.gl = $(PKGLIBS.opengl32) $(PKGLIBS.glu32)
	# mintaro
	PKGCFLAGS.mintaro =
	PKGLIBS.mintaro = -lgdi32
	# curses
	PKGCFLAGS.curses =
	PKGLIBS.curses = -lpdcurses
	# sdl2-static
	PKGCFLAGS.sdl2-static =
	PKGLIBS.sdl2-static = -L/usr/local/x86_64-w64-mingw32/lib -lmingw32 -Wl,-static -lSDL2main -lSDL2 -Wl,-Bdynamic -mwindows -Wl,--no-undefined -lm -ldinput8 -ldxguid -ldxerr8 -luser32 -lgdi32 -lwinmm -limm32 -lole32 -loleaut32 -lshell32 -lversion -luuid -static-libgcc	
	# sdl2 (dynamic)
	PKGCFLAGS.sdl2 =
	PKGLIBS.sdl2 = -lmingw32 -lSDL2main -lSDL2
else ifeq ($(OS),Linux)
	# mintaro
	PKGLIBS.mintaro = -lX11 -lXext -lasound -lpthread -lm
	PKGCLFAGS.mintaro =
	# Libraries for Curses
	PKGLIBS.curses = $(shell pkg-config --libs ncurses)
	PKGCFLAGS.curses = $(shell pkg-config --cflags ncurses)
	# Libraries for math
	PKGLIBS.m = -lm
	PKGCFLAGS.m =
endif

# include all targets
include $(wildcard *.mk)

##

all ::

###### generate rules #####

# QUIET=N to show everything
ifeq ($(QUIET),N)
QUIET=""
else ifneq ($(QUIET),@)
QUIET=@
endif

# generates configuration values to use later in gen-target
define process-config
$(info Processing configuration for $1 ...)
$1.SRCS ?= $1.c
$1.EXEC ?= $1$X
$1.CFLAGS ?= $(CFLAGS)
$1.LDFLAGS ?= $(LDFLAGS)
endef

# generates rules for one target. currently does .c to executable without intermediate .o
define gen-target
$(info Generating rules for $1 ... $($1.EXEC) : $($1.SRCS))
$($1.EXEC) : $($1.SRCS)
	$(CC) -o $$@ $$^ $($1.CFLAGS) $(if $($1.PKGS),$(call pkg-config-cflags,$($1.PKGS))) $($1.LDFLAGS) $(if $($1.PKGS),$(call pkg-config-libs,$($1.PKGS)))
all :: $($1.EXEC)
clean ::
	$(RM) $$($1.EXEC) $$($1.OBJS)
endef

ifeq ($(OS),Windows_NT)
%.res : %.rc
	$(WINDRES) $< -O coff -o $@
endif

$(eval $(foreach target,$(TARGETS),$(call process-config,$(target))))

$(eval $(foreach target,$(TARGETS),$(call gen-target,$(target))))

# $(info TARGETS=$(TARGETS))
##### END #####

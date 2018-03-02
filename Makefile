## OS detection
ifeq ($(OS),)
OS := $(shell uname -s)
endif

## Platform specific options
ifeq ($(OS),Windows_NT)
	X = .exe
	RM = del
	CC = gcc
	CFLAGS += -mwindows
	## TODO: look up packages through PKGCFLAGS/PKGLIBS. don't use pkg-config.
	pkg-config-cflags = $(foreach pkg,$(1),$$(PKGCFLAGS.$(pkg)))
	pkg-config-libs = $(foreach pkg,$(1),$$(PKGLIBS.$(pkg)))
	CFLAGS = -mwindows -Wall -W -Og -ggdb
else ifeq ($(OS),Linux)
	## TODO: first look up packages through PKGCFLAGS/PKGLIBS. then fall back to pkg-config.
	pkg-config-cflags = $(shell pkg-config --cflags $(1))
	pkg-config-libs = $(shell pkg-config --libs $(1))
	CC = gcc
	CFLAGS = -Wall -W -Og -ggdb
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
	# mintaro
	PKGCFLAGS.mintaro =
	PKGLIBS.mintaro = -lgdi32
	# curses
	PKGCFLAGS.curses =
	PKGLIBS.curses = -lpdcurses
else ifeq ($(OS),Linux)
	# mintaro
	PKGLIBS.mintaro = -lX11 -lXext -lasound -lpthread -lm
	PKGCLFAGS.mintaro =
	# Libraries for Curses
	PKGLIBS.curses = $shell pkg-config --libs ncurses)
	PKGCFLAGS.curses = $(shell pkg-config --cflags ncurses)
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
$1.CFLAGS ?= $(CFLAGS) $(if $($1.PKGS),$(call pkg-config-cflags,$($1.PKGS)))
$1.LDFLAGS ?= $(LDFLAGS) $(if $($1.PKGS),$(call pkg-config-libs,$($1.PKGS)))
endef

# generates rules for one target. currently does .c to executable without intermediate .o
define gen-target
$(info Generating rules for $1 ... $($1.EXEC) : $($1.SRCS))
$($1.EXEC) : $($1.SRCS)
	$(CC) -o $$@ $$^ $($1.CFLAGS) $($1.LDFLAGS)
all :: $($1.EXEC)
clean ::
	$(RM) $$($1.EXEC) $$($1.OBJS)
endef

$(eval $(foreach target,$(TARGETS),$(call process-config,$(target))))

$(eval $(foreach target,$(TARGETS),$(call gen-target,$(target))))

# $(info TARGETS=$(TARGETS))
##### END #####

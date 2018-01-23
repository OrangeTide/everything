all::
clean::
.PHONY: all clean
.SECONDARYEXPANSION:

# Tool settings ...
CC=gcc

CFLAGS=-std=gnu99 -Wall -W
CFLAGS+=-g

# QUIET=N to show everything
ifeq ($(QUIET),N)
QUIET=""
else ifneq ($(QUIET),@)
QUIET=@
endif

ifeq ($(OS),Windows_NT)
# Windows

# Extension for executables
X=.exe

# Definition for OS-specific variables
OSDEF=windows

# Tools
RM=del

# Libraries for mintaro
MINTARO_LDLIBS=-lgdi32
MINTARO_CFLAGS=

# Libraries for Curses
CURSES_LDLIBS=-lpdcurses
CURSES_CFLAGS=

else
# Linux/FreeBSD/etc

# Extension for executables
X=""

# Definition for OS-specific variables
OSDEF=windows

# Libraries for mintaro
MINTARO_LDLIBS=-lX11 -lXext -lasound -lpthread -lm
MINTARO_CFLAGS=

# Libraries for Curses
CURSES_LDLIBS=$(shell pkg-config --libs ncurses)
CURSES_CFLAGS=$(shell pkg-config --cflags ncurses)
endif

## Rules

# Linking executables
%$X : %.o
	@echo Linking $@ ... $^
	$(QUIET)$(LINK.o) $^ $(LOADLIBES) $(LDLIBS) -o $@

# Compiling executables
%$X : %.c
	@echo Compiling $@ ... $^
	$(QUIET)$(LINK.c) $^ $(LOADLIBES) $(LDLIBS) -o $@

# Compiling objects
%.o : %.c
	@echo Compiling $@ ... $^
	$(QUIET)$(COMPILE.c) $(OUTPUT_OPTION) $<

# Pick up all targets
include $(wildcard *.mk)
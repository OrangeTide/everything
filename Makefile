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
Q:=""
else ifeq ($(QUIET),n)
Q:=
else ifneq ($(QUIET),@)
Q:=@
endif

ifeq ($(OS),Windows_NT)
# Windows
LDFLAGS += -mwindows

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
OSDEF=linux

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
	$(Q)$(LINK.o) $^ $(LOADLIBES) $(LDLIBS) -o $@

# Compiling executables
%$X : %.c
	@echo Compiling $@ ... $^
	$(Q)$(LINK.c) $^ $(LOADLIBES) $(LDLIBS) -o $@

# Compiling objects
%.o : %.c
	@echo Compiling $@ ... $^
	$(Q)$(COMPILE.c) $(OUTPUT_OPTION) $<

# Pick up all targets
include $(wildcard *.mk)

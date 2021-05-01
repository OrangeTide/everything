CFLAGS = -D_GNU_SOURCE -D_POSIX_C_SOURCE=200809L -Wall -W -O -g -flto
LDFLAGS = -flto
SRCS = bard.c display.c report.c poll.c screen.c
PKGS = tinfo

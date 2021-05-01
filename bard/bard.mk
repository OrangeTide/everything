bard.CFLAGS = -D_GNU_SOURCE -D_POSIX_C_SOURCE=200809L -Wall -W -O -g -flto
bard.LDFLAGS = -flto
bard.SRCS = bard.c display.c report.c poll.c screen.c
bard.PKGS = tinfo

TARGETS += bard

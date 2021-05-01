service.SRCS = log.c service.c
service.CFLAGS = -Wall -W -Werror -O2 -flto -pthread
service.LDFLAGS = -pthread
TARGETS += service

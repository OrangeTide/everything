CFLAGS = -Wall -W -Werror -O2 -pthread
LDFLAGS = -pthread

EXEC := service
SRCS := $(wildcard *.c)
OBJS := $(SRCS:.c=.o)

all : $(EXEC)
clean :
	$(RM) $(EXEC) $(OBJS)
.PHONY : all clean
$(EXEC) : $(OBJS)
	$(CC) $(LDFLAGS) $(TARGET_ARCH) $(LDLIBS) -o $@ $^
$(OBJS) : %.o : %.c
	$(CC) $(CFLAGS) $(CPPFLAGS) -c -o $@ $<

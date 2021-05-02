## Host OS detection
ifeq ($(OS),)
	OS := $(shell uname -s)
endif

TARGET_OS ?= $(OS)

## Host specific options
ifeq ($(OS),Windows_NT)
ifeq ($(MINGW_CHOST),)
	RM := del
endif
	CC := gcc
else ifeq ($(OS),Linux)
	ifeq ($(TARGET_OS),Windows_NT)
		CC := x86_64-w64-mingw32-gcc
	else
		CC := gcc
	endif
endif

## Common/default options
CFLAGS := -ggdb -Os

## Target options
ifeq ($(TARGET_OS),Windows_NT)
	X := .exe
	CFLAGS := $(CFLAGS) -mwindows
else
	X :=
endif

## Rules

all ::
clean ::

%.o : %.c
	$(CC) $(CFLAGS) $(CPPFLAGS) -c -o $@ $<

%$X : %.o
	$(CC) -o $@ $(CFLAGS) $(CPPFLAGS) $(LDFLAGS) $^ $(LDLIBS)

%$X : %.c
	$(CC) -o $@ $(CFLAGS) $(CPPFLAGS) $(LDFLAGS) $^ $(LDLIBS)

## Targets

ifeq ($(TARGET_OS),Windows_NT)
# platform specific targets

# demo of a transparent window using DIB
windemo1$X : windemo1.c
windemo1$X : LDLIBS = -lopengl32 -lm
all :: windemo1$X
clean :: ; $(RM) windemo1$X

# demo of WS_EX_LAYERED and SetLayeredWindowAttributes
windemo2$X : windemo2.c
windemo2$X : LDLIBS = -lopengl32 -lm
all :: windemo2$X
clean :: ; $(RM) windemo2$X

endif

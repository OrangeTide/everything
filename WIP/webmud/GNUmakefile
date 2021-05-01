## Configuration

BINDIR = bin/
COMMON_CFLAGS = -Wall -W -Os -flto
COMMON_LDFLAGS = -flto

## Common

cmd.compile.c = $(CC) -c $(CFLAGS) $(CPPFLAGS) $(if $(PKGS),$(shell pkg-config --cflags $(PKGS))) $^ -o $@
cmd.link.c = $(CC) $(CFLAGS) $(CPPFLAGS) $(LDFLAGS) $(if $(PKGS),$(shell pkg-config --cflags --libs $(PKGS))) $^ $(LDLIBS) -o $@
cmd.link.o = $(CC) $(CFLAGS) $(CPPFLAGS) $(LDFLAGS) $(if $(PKGS),$(shell pkg-config --cflags --libs $(PKGS))) $^ $(LDLIBS) -o $@

## Rules

.SECONDEXPANSION:

%.o : %.c
	$(cmd.compile.c)

% : %.o
	$(cmd.link.o)

% : %.c
	$(cmd.link.c)

## Server

SERVER_EXEC = $(BINDIR)server
all :: $(SERVER_EXEC)
clean :: ; $(RM) $(SERVER_EXEC)

%/ :
	mkdir -p $@

$(SERVER_EXEC) : $(wildcard server/*.c) $$(CIVETWEB_OBJ) | $(dir $(SERVER_EXEC))
	$(cmd.link.c)
$(SERVER_EXEC) : CFLAGS = $(COMMON_CFLAGS) $(CIVETWEB_CFLAGS)
$(SERVER_EXEC) : LDFLAGS = $(COMMON_LDFLAGS) $(CIVETWEB_LDFLAGS)
$(SERVER_EXEC) : PKGS = $(CIVETWEB_PKGS)

## Civetweb

CIVETWEB_SRCS = civetweb/src/civetweb.c
CIVETWEB_OBJ = objs/civetweb.o
CIVETWEB_CFLAGS = -pthread -Icivetweb/include/
CIVETWEB_LDFLAGS = -lrt -ldl
CIVETWEB_PKGS = zlib

$(CIVETWEB_OBJ) : $(CIVETWEB_SRCS) | $(dir $(CIVETWEB_OBJ))
	$(cmd.compile.c)
$(CIVETWEB_OBJ) : CFLAGS = $(COMMON_CFLAGS) $(CIVETWEB_CFLAGS) -Icivetweb/src/ -DUSE_WEBSOCKET -DOPENSSL_API_1_1 -DNO_CGI -DNO_CACHING -DUSE_ZLIB -DUSE_IPV6 -DUSE_SERVER_STATS
# -DUSE_X_DOM_SOCKET -DDAEMONIZE -DPID_FILE=\"$(PID_FILE)\"

clean :: ; $(RM) $(CIVETWEB_OBJ)

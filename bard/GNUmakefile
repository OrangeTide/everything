# 1. Use the current directory as the exec name
# 2. Compiles all source files in this directory into a single executable
# 3. We only support C. Needs changes in order to support C++
CFLAGS ?= -Wall -W -std=gnu99
CFLAGS += -Os -flto
all ::
.PHONY : all clean
-include project.cfg
X:=$(notdir $(realpath .))
S:=$(wildcard *.c)
O=$(S:%.c=%.o)
D=$(S:%.c=%.d)
$O : %.o : %.c
	$(CC) $(CFLAGS) $(CPPFLAGS) $(if $(PKGS),$(shell pkg-config --cflags $(PKGS))) -MMD -c -o $@ $<
$X : % : %.o
	$(CC) $(LDFLAGS) $^ $(LOADLIBES) $(LDLIBS) $(if $(PKGS),$(shell pkg-config --libs $(PKGS))) -o $@
$X : $O
all :: $X
$(eval clean :: ; $$(RM) $X $O $D)
$(eval clean-all :: clean ; $$(RM) $D)
-include $D

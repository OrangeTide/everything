# 1. Use the current directory as the exec name
# 2. Compiles all source files in this directory into a single executable
# 3. We only support C. Needs changes in order to support C++
CFLAGS ?= -Wall -W -std=gnu99
CFLAGS += -Os -flto
all ::
test ::
.PHONY : all test clean
#### Settings
# put main executable in bin/
BINDIR := bin
# put main library in lib/
LIBDIR := lib
# hold intermediate objects and generated files in this directory
BUILDDIR := build
# selects the build type: lib or exe
TYPE := lib
#### Common Rules & Variables
%/ :
	mkdir -p $@
# ignore files that start with test_
S:=$(filter-out test_%,$(wildcard *.c))
O=$(patsubst %.c,$(if $(BUILDDIR),$(BUILDDIR)/)%.o,$S)
D=$(patsubst %.c,$(if $(BUILDDIR),$(BUILDDIR)/)%.d,$S)
$O : $(if $(BUILDDIR),$(BUILDDIR)/)%.o : %.c | $(dir $O) ; $(CC) $(CFLAGS) $(CPPFLAGS) -MMD -c -o $@ $<
#### Type specific rules
ifeq ($(TYPE),lib)
## Libraries
L:=$(if $(LIBDIR),$(LIBDIR)/)lib$(notdir $(realpath .)).a
$L : $O | $(dir $L) ; $(AR) $(ARFLAGS) $@ $^
all :: $L
$(eval clean :: ; $$(RM) $L)
else ifeq ($(TYPE),exe)
## Executables
X:=$(if $(BINDIR),$(BINDIR)/)$(notdir $(realpath .))
$X : $(if $(BINDIR),$(BINDIR)/)% : $(if $(BUILDDIR),$(BUILDDIR)/)%.o ; $(CC) $(LDFLAGS) $^ $(LOADLIBES) $(LDLIBS) -o $@
$(eval clean :: ; $$(RM) $X)
# build executable from .o files and create BINDIR, if necessary
$X : $O | $(dir $X)
all :: $X
else
$(error TYPE must be one of: exe lib)
endif
#### More common rules
$(eval clean :: ; $$(RM) $O $D)
$(eval clean-all :: clean ; $$(RM) $D ; rmdir $(BUILDDIR) $(BINDIR) $(LIBDIR))
## Tests - everything that starts with test_ is a stand-alone test.
TESTS:=$(patsubst %.c,%,$(wildcard test_*.c))
test :: $(TESTS)
# TODO: run tests?
$(eval clean :: ; $$(RM) $(TESTS))
-include $D

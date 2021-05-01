all ::
clean ::
clean-all :
.PHONY : all clean clean-all
################################################################################
## Usage: $(call pkg_config_uses,yourpackage)
define pkg_config_uses
ifneq ($1,) # ignore empty $(yourprogram.PKG)
ifeq ($$(PKG.cflags.$1),)
PKG.cflags.$1 := $$(shell pkg-config --cflags $1)
endif
ifeq ($$(PKG.libs.$1),)
PKG.libs.$1 := $$(shell pkg-config --libs $1)
endif
endif
endef
################################################################################
# clear variables
define clear-vars
undefine CFLAGS
undefine CPPFLAGS
undefine LDFLAGS
undefine LDLIBS
undefine SRCS
undefine OBJS
undefine PKGS
endef
# apply current variables
define apply
$1.CFLAGS := $$(CFLAGS)
$1.CPPFLAGS := $$(CPPFLAGS)
$1.LDFLAGS := $$(LDFLAGS)
$1.LDLIBS := $$(LDLIBS)
$1.SRCS := $$(SRCS)
$1.OBJS := $$(OBJS)
$1.PKGS := $$(PKGS)
TARGETS += $1
endef
# Load settings for each *.mk
define load
$(eval
$(call clear-vars)
include $1
$(call apply,$(1:.mk=))
)
endef
################################################################################
# Generate rules for building a simple executable.
# inputs: yourprogram.SRCS yourprogram.OBJS, yourprogram.OBJS.noclean, yourprogram.PKGS, yourprogram.CFLAGS, yourprogram.LDFLAGS
define build
$(eval
$(call pkg_config_uses,$($1.PKGS))
$1.OBJS._generated := $(patsubst %.S,%.o,$(patsubst %.s,%.o,$(patsubst %.C,%.o,$(patsubst %.cpp,%.o,$(patsubst %.cc,%.o,$(patsubst %.c,%.o,$($1.SRCS)))))))
all :: $1$X
$1$X : $$($1.OBJS) $$($1.OBJS.noclean) $$($1.OBJS._generated)
	$$(CC) $$(LDFLAGS) $$(TARGET_ARCH) $$^ $$(LOADLIBES) $$(LDLIBS) -o $$@
$1$X : CFLAGS = $($1.CFLAGS) $(foreach p,$($1.PKGS),$$(PKG.cflags.$p))
$1$X : LDFLAGS = $($1.LDFLAGS) $(foreach p,$($1.PKGS),$$(PKG.libs.$p))
clean :: ; $(RM) $1$X $($1.OBJS) $$($1.OBJS._generated)
)
endef
################################################################################
TARGET_MAKEFILES=$(wildcard *.mk)
$(foreach m,$(TARGET_MAKEFILES),$(call load,$m))
$(eval $(call clear-vars))
$(foreach t,$(TARGETS),$(call build,$t))

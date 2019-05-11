ifeq ($(HAS_SDL),1)
cgatext.SRCS = cgatext-sdl.c cgatext.c
cgatext.PKGS = sdl2
else ifeq ($(OS),Windows_NT)
cgatext.SRCS = cgatext-gdi.c cgatext.c
else ifeq ($(OS),Linux)
cgatext.SRCS = cgatext-tinfo.c cgatext.c
cgatext.PKGS = tinfo
endif

# TODO: make into a buildable static library
# TARGETS += cgatext
# PKGLIBS.cgatext =
# PKGCFLAGS.cgatext =

test_cgatext.PKGS = $(cgatext.PKGS)
test_cgatext.SRCS = $(cgatext.SRCS) test_cgatext.c
TARGETS += test_cgatext

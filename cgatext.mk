ifeq ($(OS),Windows_NT)
cgatext.SRCS = cgatext-gdi.c cgatext.c
# TODO: TARGETS += cgatext
else ifeq ($(OS),Linux)
cgatext.SRCS = cgatext-sdl.c cgatext.c
cgatext.PKGS = sdl2
# TODO: TARGETS += cgatext
# TODO: PKGLIBS.cgatext =
# TODO: PKGCFLAGS.cgatext =
endif

test_cgatext.PKGS = $(cgatext.PKGS)
test_cgatext.SRCS = $(cgatext.SRCS) test_cgatext.c
TARGETS += test_cgatext

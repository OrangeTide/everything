ifeq ($(HAS_SDL),1)
test_cgatext.SRCS = cgatext-sdl.c cgatext.c test_cgatext.c
test_cgatext.PKGS = sdl2
else ifeq ($(OS),Windows_NT)
test_cgatext.SRCS = cgatext-gdi.c cgatext.c test_cgatext.c
else ifeq ($(OS),Linux)
test_cgatext.SRCS = cgatext-tinfo.c cgatext.c test_cgatext.c
test_cgatext.PKGS = tinfo
endif

TARGETS += test_cgatext

ifeq ($(OS),Windows_NT)
# Windows-only taget
tile.SRCS = tile.c tile.res
TARGETS += tile
endif

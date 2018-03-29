ifeq ($(OS),Windows_NT)
# Windows-only taget
tile.SRCS = tile.c
TARGETS += tile
endif

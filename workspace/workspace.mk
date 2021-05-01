ifeq ($(OS),Windows_NT)
# Windows-only taget
workspace.SRCS = workspace.c
TARGETS += workspace
endif

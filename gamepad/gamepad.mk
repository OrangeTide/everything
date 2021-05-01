# Linux-only target
ifeq ($(OS),Linux)
	gamepad.SRCS = gamepad.c
	TARGETS += gamepad
endif

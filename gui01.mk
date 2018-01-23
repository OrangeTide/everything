ifeq ($(OS),Windows_NT)
OBJS.gui01.windows = drv_gdi.o
CFLAGS.gui01.windows =
LDLIBS.gui01.windows = -lgdi32
else
OBJS.gui01.linux = drv_xlib.o
CFLAGS.gui01.linux =
LDLIBS.gui01.linux = -lX11
endif
OBJS.gui01 = gui01.o
EXEC.gui01 = gui01$X
$(EXEC.gui01) : CFLAGS += $(CFLAGS.gui01.$(OSDEF))
$(EXEC.gui01) : LDLIBS += $(LDLIBS.gui01.$(OSDEF))
$(EXEC.gui01) : $(OBJS.gui01) $(OBJS.gui01.$(OSDEF))
clean :: ; $(RM) $(EXEC.gui01) $(OBJS.gui01) $(OBJS.gui01.$(OSDEF))
all :: $(EXEC.gui01)

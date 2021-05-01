.PHONY : all clean

LIBDIR = build/lib
#BINDIR = bin
#OBJDIR = build/obj
#SRCDIR = src

vpath lib%.a $(LIBDIR)

# create directories
%/ : ; mkdir -p $@

CFLAGS = -Wall -W
CPPFLAGS = -DNDEBUG
ARFLAGS = rvU

all : interp
interp : main.o -linterp ; $(CC) $(LDFLAGS) -o $@ $^ $(LDLIBS)

OBJS.libinterp =interp.o
$(LIBDIR)/libinterp.a : $(LIBDIR)/libinterp.a($(OBJS.libinterp))
$(LIBDIR)/libinterp.a($(OBJS.libinterp)) : $(LIBDIR)/
clean : ; $(RM) interp main.o $(LIBDIR)/libinterp.a $(OBJS.libinterp)

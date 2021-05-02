CC = gcc
WINDRES = windres
# RM = del

borispc.O := borispc.o editor.o tabview.o glview.o resource.o
all :: borispc
clean :: ; $(RM) borispc $(borispc.O)
borispc :  $(borispc.O)
borispc : CFLAGS = -W -Wall -mwindows -Os -g
borispc : LDLIBS = -lgdi32
resource.o : resource.rc
	$(WINDRES) $(CPPFLAGS) -o $@ $^
	
all :: mdidemo
clean :: ; $(RM) mdidemo.exe mdidemo.o resource.o
mdidemo : mdidemo.o resource.o
mdidemo : CFLAGS = -W -Wall -mwindows -Os -g

all :: demo2
clean :: ; $(RM) demo2.exe demo2.o resource.o
demo2 : demo2.o resource.o
demo2 : CFLAGS = -W -Wall -mwindows -Os -g

all :: demo3
clean :: ; $(RM) demo3.exe demo3.o resource.o
demo3 : demo3.o resource.o
demo3 : CFLAGS = -W -Wall -mwindows -Os -g
demo3 : LDLIBS = -lComctl32

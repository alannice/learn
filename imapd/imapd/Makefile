
###########################################################################

TARGET=imapd
BINDIR=/usr/local/bin
INSTALL=install -m 0644 -o root -g wheel 

CC=clang
CFLAGS= -ggdb -I../../libxyz
LDFLAGS=
LIBS= -lssl


SOURCE=imapd.c setting.c
HEADER=imapd.h setting.h

OBJECT=imapd.o setting.o
ADDOBJ=../../libxyz/libxyz.a

.PHONY:all 

all: $(OBJECT) $(TARGET)

$(TARGET):$(OBJECT) 
	$(CC) $(CFLAGS) $(OBJECT) $(ADDOBJ) -o $(TARGET) $(LDFLAGS) $(LIBS)

$(OBJECT):$(SOURCE) $(HEADER)

clean:
	rm -f $(OBJECT) $(TARGET)

install:
	$(INSTALL) $(TARGET) $(BINDIR)/$(TARGET)

uninstall:
	rm -f $(BINDIR)/$(TARGET)
	

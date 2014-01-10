
###########################################################################

TARGET=imapd
BINDIR=/usr/local/bin
INSTALL=install -m 0644 -o root -g wheel 

CC=gcc
CFLAGS= -ggdb -I../../libxyz
LDFLAGS=
LIBS= -lssl


SOURCE=imapd.c setting.c client.c
HEADER=imapd.h setting.h client.h

OBJECT=imapd.o setting.o client.o
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
	

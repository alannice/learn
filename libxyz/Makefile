
###########################################################################

MYLIB=libzc
MYVER=1.0.0
TARGET=$(MYLIB)-$(MYVER).so
INCDIR=/usr/local/include/zc
LIBDIR=/usr/local/lib
INSTALL=install -m 0644 -o root -g wheel 

CC=gcc
CFLAGS= -fPIC
LDFLAGS=
LIBS=


SOURCE=buf.c  conf.c  event.c  list.c  log.c  mpool.c  sock.c 
HEADER=buf.h  conf.h  event.h  list.h  log.h  mpool.h  sock.h

OBJECT=buf.o conf.o event.o list.o log.o mpool.o sock.o 

.PHONY:all 

all: $(OBJECT) $(TARGET)

$(TARGET):$(OBJECT) 
	$(CC) $(CFLAGS) -shared $(OBJECT) -Wl,-soname -Wl,$(TARGET)  -o $(TARGET) $(LDFLAGS) $(LIBS)

$(OBJECT):$(SOURCE) $(HEADER)

clean:
	rm -f *.o

install:
	rm -rf $(INCDIR) && mkdir $(INCDIR)
	$(INSTALL)  buf.h  $(INCDIR)/buf.h
	$(INSTALL)  conf.h $(INCDIR)/conf.h
	$(INSTALL)  event.h $(INCDIR)/event.h
	$(INSTALL)  list.h  $(INCDIR)/list.h
	$(INSTALL)  log.h   $(INCDIR)/log.h
	$(INSTALL)  mpool.h $(INCDIR)/mpool.h
	$(INSTALL)  sock.h  $(INCDIR)/sock.h
	$(INSTALL)  $(TARGET) $(LIBDIR)/$(TARGET)
	rm -f $(LIBDIR)/$(MYLIB).so 
	ln -s $(LIBDIR)/$(TARGET) $(LIBDIR)/$(MYLIB).so
	

uninstall:
	rm -rf $(INCDIR)
	rm -f $(LIBDIR)/$(MYLIB).so
	rm -f $(LIBDIR)/$(TARGET)
	

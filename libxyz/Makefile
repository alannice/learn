
###########################################################################

MYLIB=libxyz
MYVER=1.0.0
TARGET=$(MYLIB)-$(MYVER).so
INCDIR=/usr/local/include/xyz
LIBDIR=/usr/local/lib
INSTALL=install -m 0644 -o root -g wheel 

TARGETA=$(MYLIB).a
AR=ar
ARFLAGS= -crs

CC=cc
CFLAGS= -ggdb -fPIC -Wall -Werror
LDFLAGS=
LIBS= -lssl


SOURCE=xyz_buf.c  xyz_conf.c  xyz_event.c  xyz_list.c  xyz_log.c  xyz_mpool.c  xyz_sock.c xyz_ssl.c \
xyz_hash.c xyz_plugin.c xyz_netmodel.c xyz_event2.c
HEADER=xyz_buf.h  xyz_conf.h  xyz_event.h  xyz_list.h  xyz_log.h  xyz_mpool.h  xyz_sock.h xyz_ssl.h \
xyz_hash.h xyz_plugin.h xyz_netmodel.h xyz_event2.c

OBJECT=xyz_buf.o xyz_conf.o xyz_event.o xyz_list.o xyz_log.o xyz_mpool.o xyz_sock.o xyz_ssl.o \
xyz_hash.o xyz_plugin.o xyz_netmodel.o xyz_event2.o

.PHONY:all 

all: $(OBJECT) $(TARGET)

$(TARGET):$(OBJECT) 
	$(CC) $(CFLAGS) -shared $(OBJECT) -Wl,-soname -Wl,$(TARGET)  -o $(TARGET) $(LDFLAGS) $(LIBS)
	$(AR) $(ARFLAGS) $(TARGETA) $(OBJECT) $(LDFLAGS) 

$(OBJECT):$(SOURCE) $(HEADER)

clean:
	rm -f *.o

install:
	rm -rf $(INCDIR) && mkdir $(INCDIR)
	$(INSTALL)  xyz_buf.h  $(INCDIR)/xyz_buf.h
	$(INSTALL)  xyz_conf.h $(INCDIR)/xyz_conf.h
	$(INSTALL)  xyz_event.h $(INCDIR)/xyz_event.h
	$(INSTALL)  xyz_list.h  $(INCDIR)/xyz_list.h
	$(INSTALL)  xyz_log.h   $(INCDIR)/xyz_log.h
	$(INSTALL)  xyz_mpool.h $(INCDIR)/xyz_mpool.h
	$(INSTALL)  xyz_sock.h  $(INCDIR)/xyz_sock.h
	$(INSTALL)  $(TARGET) $(LIBDIR)/$(TARGET)
	rm -f $(LIBDIR)/$(MYLIB).so 
	ln -s $(LIBDIR)/$(TARGET) $(LIBDIR)/$(MYLIB).so
	

uninstall:
	rm -rf $(INCDIR)
	rm -f $(LIBDIR)/$(MYLIB).so
	rm -f $(LIBDIR)/$(TARGET)
	


###########################################################################

TARGET=libimap-cmd.a
AR=ar
ARFLAGS= -crs

CC=gcc
CFLAGS= -ggdb -fPIC -I../../libxyz

SOURCE= cmd-append.c       cmd-copy.c         cmd-fetch.c        cmd-noop.c         cmd-subscribe.c \
		cmd-authenticate.c cmd-create.c       cmd-list.c         cmd-rename.c       cmd-uid.c \
		cmd-capability.c   cmd-delete.c       cmd-login.c        cmd-select.c       cmd-unsubscribe.c \
		cmd-check.c        cmd-examine.c      cmd-logout.c       cmd-status.c       command.c \
		cmd-close.c        cmd-expunge.c      cmd-lsub.c         cmd-store.c        cmd-search.c \
		cmd-id.c
HEADER= command.h

OBJECT= cmd-append.o       cmd-copy.o         cmd-fetch.o        cmd-noop.o         cmd-subscribe.o \
		cmd-authenticate.o cmd-create.o       cmd-list.o         cmd-rename.o       cmd-uid.o \
		cmd-capability.o   cmd-delete.o       cmd-login.o        cmd-select.o       cmd-unsubscribe.o \
		cmd-check.o        cmd-examine.o      cmd-logout.o       cmd-status.o       command.o \
		cmd-close.o        cmd-expunge.o      cmd-lsub.o         cmd-store.o        cmd-search.o \
		cmd-id.o

.PHONY:all 

all: $(OBJECT) $(TARGET)

$(TARGET):$(OBJECT) 
	$(AR) $(ARFLAGS) $(TARGET) $(OBJECT) 

$(OBJECT):$(SOURCE) $(HEADER)

clean:
	rm -f *.o



###########################################################################

TARGET=libimap-parser.a
AR=ar
ARFLAGS= -crs

CC=gcc
CFLAGS= -ggdb -fPIC 

SOURCE= args-parser.c
HEADER= args-parser.h

OBJECT= args-parser.o

.PHONY:all 

all: $(OBJECT) $(TARGET)

$(TARGET):$(OBJECT) 
	$(AR) $(ARFLAGS) $(TARGET) $(OBJECT) 

$(OBJECT):$(SOURCE) $(HEADER)

clean:
	rm -f *.o


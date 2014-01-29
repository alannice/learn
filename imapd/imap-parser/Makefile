
###########################################################################

TARGET=libimap-parser.a
AR=ar
ARFLAGS= -crs

CC=clang
CFLAGS= -ggdb -fPIC 

SOURCE= args-parser.c addr-parser.c
HEADER= args-parser.h addr-parser.h

OBJECT= args-parser.o addr-parser.o

.PHONY:all 

all: $(OBJECT) $(TARGET)

$(TARGET):$(OBJECT) 
	$(AR) $(ARFLAGS) $(TARGET) $(OBJECT) 

$(OBJECT):$(SOURCE) $(HEADER)

clean:
	rm -f *.o


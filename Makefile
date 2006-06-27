CFLAGS = -Wall -O2 
LDFLAGS = 
OBJS = main.o config.o device.o serial.o
BIN = seprog

# CC = i386-mingw32-gcc
# STRIP = i386-mingw32-strip
# CFLAGS += -DWIN32

$(BIN):	$(OBJS)
	$(CC) $(LDFLAGS) $(OBJS) -o $@

#$(BIN).exe:	$(OBJS)
#	$(CC) $(LDFLAGS) $(OBJS) -o $@
#	$(STRIP) $@

.PHONY:	clean install

clean:
	rm -f *.o *~ core $(BIN) $(BIN).exe

install:
	install $(BIN) $(DESTDIR)/usr/local/bin


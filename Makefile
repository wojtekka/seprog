CC = gcc
CFLAGS = -Wall -O2 
LDFLAGS = 
BIN = seprog
OBJS = main.o config.o device.o serial.o
ALL = $(BIN)

WINCC = i386-mingw32-gcc
WINCFLAGS = -Wall -O2 -DWIN32
WINLDFLAGS = 
WINOBJS = $(OBJS:.o=.obj)
WINBIN = $(BIN).exe
WINSTRIP = i386-mingw32-strip

ifneq ($(wildcard /usr/bin/$(WINCC)),)
ALL += $(WINBIN)
endif

all:	$(ALL)

$(BIN):	$(OBJS)
	$(CC) $(LDFLAGS) $(OBJS) -o $@

%.obj:	%.c
	$(WINCC) $(WINCFLAGS) -c $< -o $@

$(WINBIN):	$(WINOBJS)
	$(WINCC) $(WINLDFLAGS) $(WINOBJS) -o $@
	$(WINSTRIP) $@

.PHONY:	clean install

clean:
	rm -f *.o *.obj *~ core $(BIN) $(WINBIN)

install:
	install $(BIN) $(DESTDIR)/usr/local/bin


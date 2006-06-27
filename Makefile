CFLAGS = -Wall -O2 
OBJS = serial.o main.o config.o device.o
BIN = seprog

$(BIN):	$(OBJS)
	$(CC) $(OBJS) -o $(BIN)

.PHONY:	clean install

clean:
	rm -f *.o *~ core $(BIN)

install:
	install $(BIN) $(DESTDIR)/usr/local/bin


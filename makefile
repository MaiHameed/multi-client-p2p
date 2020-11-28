CC=gcc
CFLAGS=-lnsl
BINDIR = bin

make : server.c
	mkdir -p bin
	$(CC) -o bin/server server.c $(CFLAGS)

.PHONY: clean

clean:
	rm -f $(BINDIR)/*
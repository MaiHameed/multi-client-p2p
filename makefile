CC=gcc
CFLAGS=-lnsl
BINDIR = bin

server : server.c 
	mkdir -p bin
	$(CC) -o bin/server server.c $(CFLAGS)

client: client.c
	mkdir -p bin
	$(CC) -o bin/client client.c $(CFLAGS)

.PHONY: clean

clean:
	rm -f $(BINDIR)/*

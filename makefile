CC=gcc
CFLAGS=-lnsl
BINDIR = bin

server : server.c 
	$(CC) -o server server.c set.c $(CFLAGS)

client: client.c
	$(CC) -o client client.c set.c $(CFLAGS)

client2: client2.c
	$(CC) -o client2 client2.c set.c $(CFLAGS)
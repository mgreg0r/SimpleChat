CFLAGS=-std=gnu99 -Wall

all : client.c server.c
	gcc $(CFLAGS) -o client client.c
	gcc $(CFLAGS) -o server server.c

client : client.c
	gcc $(CFLAGS) -o client client.c

server : server.c
	gcc $(CFLAGS) -o server server.c

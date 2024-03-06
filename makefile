CC = gcc

all: clean client server

clean: 
	rm -f -r client server *.log *.dSYM

client: client.c
	$(CC) -o client client.c

server: server.c
	$(CC) -o server server.c


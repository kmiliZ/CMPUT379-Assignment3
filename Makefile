CC = g++
OBJ = client commands.o server
CFLAGS= -O -Wall -pthread

all: $(OBJ)

client:	client.cpp commands.o
	$(CC) $(CFLAGS) client.cpp commands.o -o client

server:	server.cpp commands.o
	$(CC) $(CFLAGS) server.cpp commands.o -o server

commands.o:
	$(CC) $(CFLAGS) -c commands.cpp
clean:
	rm -f $(OBJ)


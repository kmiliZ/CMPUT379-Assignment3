CC = g++
OBJ = client commands.o server helper.o
CFLAGS= -O -Wall -pthread
C_OBJ = client.cpp commands.o helper.o
S_OBJ = server.cpp commands.o helper.o
all: $(OBJ)

client:	$(C_OBJ)
	$(CC) $(CFLAGS) $(C_OBJ) -o client

server:	$(S_OBJ)
	$(CC) $(CFLAGS) $(S_OBJ) -o server

commands.o:
	$(CC) $(CFLAGS) -c commands.cpp

helper.o:
	$(CC) $(CFLAGS) -c helper.cpp
clean:
	rm -f $(OBJ)
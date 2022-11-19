CC = g++
OBJ = client commands.o server helper.o
PDF = client.pdf server.pdf
CFLAGS= -O -Wall -pthread
C_OBJ = client.cpp commands.o helper.o
S_OBJ = server.cpp commands.o helper.o
all: $(OBJ) $(PDF)

client:	$(C_OBJ)
	$(CC) $(CFLAGS) $(C_OBJ) -o client

server:	$(S_OBJ)
	$(CC) $(CFLAGS) $(S_OBJ) -o server

commands.o:
	$(CC) $(CFLAGS) -c commands.cpp

helper.o:
	$(CC) $(CFLAGS) -c helper.cpp

client.pdf:
	groff -Tpdf -man client.man > client.pdf

server.pdf:
	groff -Tpdf -man server.man > server.pdf	
clean:
	rm -f $(OBJ) $(PDF)
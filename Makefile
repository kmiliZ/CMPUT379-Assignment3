all:	client server

client:	client.cpp
	gcc -O client.cpp -o client

server:	server.cpp
	gcc -O server.cpp -o server

clean:
	rm -f client server

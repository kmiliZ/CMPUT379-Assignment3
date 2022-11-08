// From: www.thegeekstuff.com/2011/12/c-socket-programming
// Note that port# 5002 is hard-coded into this implementation

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <time.h>
#include "header.h"

// TODO: execute the revied transaction task
// TODO: log transationcs in file
// deal with 30 secs od no incoming messages, the server should exit
int main(int argc, char *argv[])
{
    int listenfd = 0, connfd = 0;
    struct sockaddr_in serv_addr;
    int readBytes, writeBytes;

    char sendBuff[1024];
    char buffer[1024];

    listenfd = socket(AF_INET, SOCK_STREAM, 0);
    if (listenfd < 0)
    {
        perror("Socket failtd");
        exit(EXIT_FAILURE);
    }
    memset(&serv_addr, '0', sizeof(serv_addr));
    memset(sendBuff, '0', sizeof(sendBuff));

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_addr.sin_port = htons(5002);

    if (bind(listenfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
    {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    if (listen(listenfd, 10) < 0)
    {
        perror("listen");
        exit(EXIT_FAILURE);
    }

    while (1)
    {
        printf("wait for a connection\n");
        connfd = accept(listenfd, (struct sockaddr *)NULL, NULL);
        if (connfd < 0)
        {
            perror("Accept");
            exit(EXIT_FAILURE);
        }

        printf("connection made\n");

        // TODO: read the actual task and n, log to file
        readBytes = read(connfd, buffer, 1024);
        printf("data read: %d bytes\n", readBytes);
        printf("Received from server: %s\n", buffer);

        // TODO: 123 just a place holder. should be the actaul number of n which was recieved from client
        snprintf(sendBuff, sizeof(sendBuff), "%d", 123);

        writeBytes = send(connfd, sendBuff, strlen(sendBuff), 0);
        printf("write to client:finished");

        close(connfd);
        sleep(1);
    }
}

// From: www.thegeekstuff.com/2011/12/c-socket-programming
// Note that port# 5002 is hard-coded into this implementation

// TODO: send T and number n to server
// TODO: log transactions and recieving events in file

#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <arpa/inet.h>
#include "header.h"

int main(int argc, char *argv[])
{
    int sockfd = 0, n = 0;
    char recvBuff[1024];
    int bytes;
    struct sockaddr_in serv_addr;

    if (argc != 2)
    {
        printf("\n Usage: %s <ip of server> \n", argv[0]);
        return 1;
    }

    memset(recvBuff, '0', sizeof(recvBuff));
    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        printf("\n Error : Could not create socket \n");
        return 1;
    }

    memset(&serv_addr, '0', sizeof(serv_addr));

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(5002);

    if (inet_pton(AF_INET, argv[1], &serv_addr.sin_addr) <= 0)
    {
        printf("\n inet_pton error occured\n");
        return 1;
    }

    if (connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
    {
        printf("\n Error : Connect Failed \n");
        return 1;
    }
    char command_type;
    int command_n;
    char sendBuff[1024];
    while (scanf("%c%u", &command_type, &command_n) > 0)
    {
        if (command_type == 'T')
        {
            printf("writing something\n");
            // send the task to server
            snprintf(sendBuff, sizeof(sendBuff), "%d %s %d\n", n, "clinet", 123);
            bytes = send(sockfd, sendBuff, strlen(sendBuff), 0);
            // TODO:log sending the serd(T num)

            printf("write to server\n");
            n = read(sockfd, recvBuff, sizeof(recvBuff) - 1);
            if (n < 0)
            {
                printf("Read Error");
            }
            // log receive to file
            printf("Received from server: %s\n", recvBuff);
        }
        else if (command_type == 'S')
        {
            // TODO: logg sleep
            printf("recied sleeping...");
            Sleep(command_n);
        }
    }
    // log the total Task send to file

    return 0;
}

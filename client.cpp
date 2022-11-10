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
#include <iostream>
#include <unistd.h>
#include <errno.h>
#include <arpa/inet.h>
#include "header.h"
#include <chrono>
using namespace std;

FILE *fp;
int totalTransactions = 0;

void logTransactionCall(char type, int n)
{
    const auto p1 = chrono::system_clock::now();
    char timeStr[15];
    sprintf(timeStr, "%.2f:", (chrono::duration_cast<chrono::milliseconds>(p1.time_since_epoch()).count()) * 0.001);
    char typeStr[] = "    ";
    type == 'T' ? sprintf(typeStr, "%s", "Send") : sprintf(typeStr, "%s", "Recv");
    fprintf(fp, "%s %s (%c%3d)\n", timeStr, typeStr, type, n);
}

void logSleep(int n)
{
    fprintf(fp, "Sleep %d units\n", n);
}

void logSummery()
{
    fprintf(fp, "Sent %d transactions\n", totalTransactions);
}

int main(int argc, char *argv[])
{
    int sockfd = 0, n = 0;
    char recvBuff[1024] = {0};
    int readBytes, writeBytes;
    struct sockaddr_in serv_addr;
    int port;

    if (argc != 3)
    {
        printf("\n Usage: %s <port number> <ip of server> \n", argv[0]);
        return 1;
    }

    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        printf("\n Error : Could not create socket \n");
        return 1;
    }
    port = atoi(argv[1]);
    memset(&serv_addr, '0', sizeof(serv_addr));

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(port);

    if (inet_pton(AF_INET, argv[2], &serv_addr.sin_addr) <= 0)
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
    char sendBuff[1024] = {'\n'};
    char hostName[100];

    // get pid
    pid_t pid = getpid();

    // Get host name
    if (gethostname(hostName, 100) < 0)
    {
        printf("gethostname failed\n");
        exit(EXIT_FAILURE);
    }
    char outPutFileName[50];
    sprintf(outPutFileName, MACHINE_NAME_FORMAT, hostName, pid);
    fp = fopen(outPutFileName, "w");

    fprintf(fp, "Using port %d\n", port);
    fprintf(fp, "Using server address %s\n", argv[2]);
    fprintf(fp, "Host %s.%d\n", hostName, pid);

    while (scanf("%c%u", &command_type, &command_n) > 0)
    {
        if (command_type == 'T')
        {
            printf("writing something\n");
            memset(sendBuff, '\0', sizeof sendBuff);

            snprintf(sendBuff, sizeof(sendBuff), MESSAGE_FORMAT, command_n, hostName, pid);
            writeBytes = send(sockfd, sendBuff, strlen(sendBuff), 0);
            if (writeBytes < 0)
            {
                printf("Send Error\n");
            }

            logTransactionCall('T', command_n);
            totalTransactions++;
            readBytes = read(sockfd, recvBuff, sizeof(recvBuff));
            int n;
            char ack[3];
            sscanf(recvBuff, "%s%d", &ack, &n);

            if (readBytes < 0)
            {
                printf("Read Error\n");
            }
            // log receive to file
            logTransactionCall('D', command_n);
            printf("Received from server: %s %d\n", ack, n);
        }
        else if (command_type == 'S')
        {
            // TODO: logg sleep
            logSleep(command_n);
            printf("recied sleeping...");
            Sleep(command_n);
        }
    }
    // log the total Task send to file
    logSummery();
    fclose(fp);
    return 0;
}

// From: www.thegeekstuff.com/2011/12/c-socket-programming
// Note that port# 5002 is hard-coded into this implementation
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <arpa/inet.h>
#include "header.h"

FILE *fp;
char hostName[100];
int totalTransactions = 0;

void logTransactionCall(char type, int n)
{
    char timeStr[15];
    getCurrentEpochTime(timeStr);

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

/*
 * Reads input entries from command line or from
 * a file until EOF.
 * Sends message to server if an entry is
 * T <n>, and reads a acknowledgement from
 * the server back.
 * If entry is S <n>, client will sleep for
 * some time. Inputs are assumes to be in
 * the right format.
 */
void run(int sockfd, pid_t pid)
{
    char recvBuff[1024] = {0};
    int readBytes, writeBytes;
    char command_type;
    int command_n;
    char sendBuff[1024] = {'\n'};
    while (scanf("%c%u", &command_type, &command_n) > 0)
    {
        if (command_type == 'T')
        {
            printf("sending a transaction to server...\n");
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
            sscanf(recvBuff, "%s%d", ack, &n);

            if (readBytes < 0)
            {
                printf("Read Error\n");
            }
            // log receive to file
            logTransactionCall('D', command_n);
        }
        else if (command_type == 'S')
        {
            logSleep(command_n);
            Sleep(command_n);
        }
    }
}

int main(int argc, char *argv[])
{
    int sockfd = 0;

    struct sockaddr_in serv_addr;
    int port;

    if (argc != 3)
    {
        printf("\n Usage: %s <port number> <ip of server> \n", argv[0]);
        return 1;
    }

    port = atoi(argv[1]);

    if (port > 64000 || port < 5000)
    {
        printf("use prot in the range 5000 to 64,000\n");
        return 1;
    }

    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        printf("\n Error : Could not create socket \n");
        return 1;
    }

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

    // reads input and send transactions to the server
    run(sockfd, pid);

    // log the total Task send to file
    logSummery();

    fclose(fp);
    return 0;
}

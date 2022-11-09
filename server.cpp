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
#include <string>
#include <map>
#include <chrono>
#include <cstring>
#include <iostream>
using namespace std;
using namespace chrono;
typedef system_clock Clock;

FILE *fp;
int taskCount;
Clock::time_point startTime;

float getTimeDuration()
{
    Clock::time_point now = Clock::now();
    duration<double> duration = now - startTime;
    return duration.count();
}

void logTaskCall(char *clientName, int n)
{
    fprintf(fp, "# %3d (T%3d) from %s\n", taskCount, n, clientName);
}
void logDone(char *clientName)
{
    fprintf(fp, "# %3d (Done) from %s\n", taskCount, clientName);
}

void logSummary(map<string, int> clientRecords)
{

    fprintf(fp, "SUMMARY\n");
    for (auto const &client : clientRecords)
    {
        fprintf(fp, "   %3d transactions from %s\n", client.second, client.first.c_str());
    }
    float duration = getTimeDuration();
    fprintf(fp, "%.1f Transactions/sec (%d/%.2f)", (float)(taskCount - 1) / (duration - 30), taskCount - 1, duration - 30);
}

int main(int argc, char *argv[])
{
    startTime = Clock::now();

    int master_socket = 0, connfd, nfds, newfd;
    int client_sockets[MAX_CLIENTS];
    struct sockaddr_in serv_addr;
    int readBytes, writeBytes;
    struct timeval timeout;
    taskCount = 1;
    map<string, int> clientRecords;

    char sendBuff[1024] = {'\0'};
    char readbuffer[1024] = {'\0'};

    if (argc != 2)
    {
        printf("\n Usage: %s <port number> \n", argv[0]);
        return 1;
    }

    int port = atoi(argv[1]);

    // set of socket descriptors
    fd_set readfds;

    string readLine;

    master_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (master_socket < 0)
    {
        perror("Socket failtd");
        exit(EXIT_FAILURE);
    }
    memset(&serv_addr, '0', sizeof(serv_addr));

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_addr.sin_port = htons(port);

    if (bind(master_socket, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
    {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    // maximum of 10 pending connections for the master socket
    if (listen(master_socket, 10) < 0)
    {
        perror("listen");
        exit(EXIT_FAILURE);
    }

    // initialise all client_socket[] to 0 so not checked
    for (int i = 0; i < MAX_CLIENTS; i++)
    {
        client_sockets[i] = 0;
    }

    timeout.tv_sec = 30;
    timeout.tv_usec = 0;
    fp = fopen("server.log", "w");
    fprintf(fp, "Using port %d\n", port);
    while (1)
    {
        printf("wait for a connection\n");

        // clear the socket set
        FD_ZERO(&readfds);

        // add master socket to set
        FD_SET(master_socket, &readfds);
        nfds = master_socket;
        // add child sockets to set
        for (int i = 0; i < MAX_CLIENTS; i++)
        {
            // socket descriptor
            connfd = client_sockets[i];

            // if valid socket descriptor then add to read list
            if (connfd > 0)
                FD_SET(connfd, &readfds);

            // highest file descriptor number, need it for the select function
            if (connfd > nfds)
                nfds = connfd;
        }

        int activity = select(nfds + 1, &readfds, NULL, NULL, &timeout);

        if (activity < 0)
        {
            perror("select");
        }
        else if (activity == 0)
        {
            printf("server timeout");
            break;
        }

        // new connection
        if (FD_ISSET(master_socket, &readfds))
        {
            if ((newfd = accept(master_socket, (struct sockaddr *)NULL, NULL)) < 0)
            {
                perror("Accept");
                exit(EXIT_FAILURE);
            }

            printf("New connection , socket fd is %d , ip is : %s , port : %d \n ", newfd, inet_ntoa(serv_addr.sin_addr), ntohs(serv_addr.sin_port));

            // TODO: read the actual task and n, log to file
            memset(readbuffer, '\0', sizeof readbuffer);
            readBytes = recv(newfd, readbuffer, 1024, 0);
            // printf("data read: %d bytes\n", readBytes);
            // printf("Received from server: %s\n", readbuffer);

            int n, pid;
            char clientName[100] = {'\0'};
            sscanf(readbuffer, "%d %s %d", &n, &clientName, &pid);
            char machineName[50];
            sprintf(machineName, "%s.%d\0", clientName, pid);
            logTaskCall(machineName, n);

            // perform work
            Trans(n);

            snprintf(sendBuff, sizeof(sendBuff), "D %d", n);

            writeBytes = send(newfd, sendBuff, strlen(sendBuff), 0);
            logDone(machineName);

            clientRecords.emplace(string(machineName), 1);
            taskCount++;

            // add new socket to array of sockets
            for (int i = 0; i < MAX_CLIENTS; i++)
            {
                // if position is empty
                if (client_sockets[i] == 0)
                {
                    client_sockets[i] = newfd;
                    break;
                }
            }
        }
        for (int i = 0; i < MAX_CLIENTS; i++)
        {
            connfd = client_sockets[i];

            if (FD_ISSET(connfd, &readfds))
            {
                // Check if it was for closing , and also read the
                // incoming message
                memset(readbuffer, '\0', sizeof readbuffer);

                readBytes = recv(newfd, readbuffer, 1024, 0);

                if (readBytes == 0)
                {
                    // Somebody disconnected , get his details and print
                    getpeername(connfd, (struct sockaddr *)&serv_addr,
                                (socklen_t *)&serv_addr);
                    printf("Host disconnected , ip %s , port %d \n",
                           inet_ntoa(serv_addr.sin_addr), ntohs(serv_addr.sin_port));

                    // Close the socket and mark as 0 in list for reuse
                    close(connfd);
                    client_sockets[i] = 0;
                }
                else if (readBytes < 0)
                {
                    perror("read");
                }
                // Echo back the message that came in
                else
                {
                    // set the string terminating NULL byte on the end
                    // of the data read
                    int n, pid;
                    char clientName[100] = {'\0'};
                    sscanf(readbuffer, "%d %s %d", &n, &clientName, &pid);
                    char machineName[50] = {'\0'};

                    sprintf(machineName, "%s.%d", clientName, pid);

                    logTaskCall(machineName, n);

                    // perform work
                    Trans(n);

                    snprintf(sendBuff, sizeof(sendBuff), "D %d", n);

                    writeBytes = send(newfd, sendBuff, strlen(sendBuff), 0);
                    logDone(machineName);

                    clientRecords.at(string(machineName))++;
                    taskCount++;
                }
            }
        }
    }
    logSummary(clientRecords);
}

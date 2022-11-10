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

void giveAcknowledgement(int n, int fd)
{
    char sendBuff[1024] = {'\0'};
    snprintf(sendBuff, sizeof(sendBuff), "D %d", n);

    int writeBytes = send(fd, sendBuff, strlen(sendBuff), 0);
    if (writeBytes < 0)
    {
        printf("send error\n");
    }
}

string performTransaction(char *readbuffer, int fd)
{
    int n, pid;
    char clientName[100] = {'\0'};
    char machineName[50] = {'\0'};
    sscanf(readbuffer, MESSAGE_FORMAT, &n, &clientName, &pid);
    sprintf(machineName, MACHINE_NAME_FORMAT, clientName, pid);

    // log recived
    logTaskCall(machineName, n);

    // perform work
    Trans(n);

    giveAcknowledgement(n, fd);
    logDone(machineName);
    taskCount++;

    return string(machineName);
}

int main(int argc, char *argv[])
{
    startTime = Clock::now();

    int master_socket = 0, connfd, nfds, newfd;
    int client_sockets[MAX_CLIENTS];
    struct sockaddr_in serv_addr;
    int readBytes;
    struct timeval timeout;
    taskCount = 1;
    map<string, int> clientRecords;

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

    fp = fopen(SERVER_OUTPUT_FILE, "w");
    fprintf(fp, "Using port %d\n", port);
    while (1)
    {
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
        timeout.tv_sec = 30;
        timeout.tv_usec = 0;
        int activity = select(nfds + 1, &readfds, NULL, NULL, &timeout);

        if (activity < 0)
        {
            perror("select error\n");
        }
        else if (activity == 0)
        {
            printf("server timeout\n");
            break;
        }

        // new connection
        if (FD_ISSET(master_socket, &readfds))
        {
            if ((newfd = accept(master_socket, (struct sockaddr *)NULL, NULL)) < 0)
            {
                perror("Accept\n");
                exit(EXIT_FAILURE);
            }
            printf("Client connected\n");

            memset(readbuffer, '\0', sizeof readbuffer);
            readBytes = recv(newfd, readbuffer, 1024, 0);

            string machineName = performTransaction(readbuffer, newfd);

            clientRecords.emplace(string(machineName), 1);

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

                readBytes = recv(connfd, readbuffer, 1024, 0);

                if (readBytes == 0)
                {
                    printf("Client disconnected\n");

                    // Close the socket and mark as 0 in list for reuse
                    close(connfd);
                    client_sockets[i] = 0;
                }
                else if (readBytes < 0)
                {
                    perror("read error\n");
                }
                // Echo back the message that came in
                else
                {
                    string machineName = performTransaction(readbuffer, connfd);
                    clientRecords.at(string(machineName))++;
                }
            }
        }
    }
    logSummary(clientRecords);
    fclose(fp);
}

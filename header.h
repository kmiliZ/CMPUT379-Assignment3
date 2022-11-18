#ifndef __CLIENT_SERVER_H_
#define __CLIENT_SERVER_H_

#define MAX_CLIENTS 30
#define SERVER_OUTPUT_FILE "server.log"
#define MACHINE_NAME_FORMAT "%s.%d"
#define MESSAGE_FORMAT "%d %s %d"

#include <string.h>
#include <chrono>
#include <iostream>
using namespace std;
using namespace chrono;

typedef system_clock Clock;
void Trans(int n);
void Sleep(int n);
void getCurrentEpochTime(char *epochStr);
float getTimeDuration(Clock::time_point startTime);

#endif
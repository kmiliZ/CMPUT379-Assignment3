#include "header.h"

void getCurrentEpochTime(char *epochStr)
{
    const auto p1 = chrono::system_clock::now();
    sprintf(epochStr, "%.2f:", (chrono::duration_cast<chrono::milliseconds>(p1.time_since_epoch()).count()) * 0.001);
}

float getTimeDuration(Clock::time_point startTime)
{
    Clock::time_point now = Clock::now();
    duration<double> duration = now - startTime;
    return duration.count();
}
#include <cstdlib>
#include <sys/time.h>
#include "defs.h"

double g_Time, g_FrameDelta;

void SYS_Sleep(int milli)
{
    struct timeval t;
    t.tv_sec = milli / 1000;
    t.tv_usec = (milli % 1000) * 1000;
    select(0, NULL, NULL, NULL, &t);
}

int SYS_GetMilli(void)
{
    int curtime;
    struct timeval tp;
    struct timezone tzp;
    static int secbase = 0;
    gettimeofday(&tp, &tzp);
    if (secbase == 0) {
        secbase = tp.tv_sec;
        return tp.tv_usec / 1000;
    }
    curtime = (tp.tv_sec - secbase) * 1000 + tp.tv_usec / 1000;
    return curtime;
}

static double gettimeseconds()
{
    return double(SYS_GetMilli()) / 1000.0;
}

static void sleepseconds(double t)
{
    SYS_Sleep(int(t * 1000.0));
}

void SYS_UpdateTime()
{
    static double LastTime = 0.0;

    if (LastTime == 0.0) {
        LastTime = gettimeseconds();
        sleepseconds(1.0/30.0);
    }
    g_Time = gettimeseconds();
    g_FrameDelta = g_Time - LastTime;
    if (g_FrameDelta < 1.0/30.0) {
        sleepseconds(1.0/30.0 - g_FrameDelta);
        g_Time = gettimeseconds();
        g_FrameDelta = g_Time - LastTime;
    }
    LastTime = g_Time;
}

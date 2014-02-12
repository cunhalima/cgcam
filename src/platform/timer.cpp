#include <cstdlib>
#ifndef _WIN32
#include <sys/time.h>
#else
#include <windows.h>
#include <mmsystem.h>
#pragma comment(lib, "winmm.lib")
#endif
#include "../defs.h"

int g_Time, g_FrameDelta;

void SYS_Sleep(int milli) {
#ifndef _WIN32
    struct timeval t;
    t.tv_sec = milli / 1000;
    t.tv_usec = (milli % 1000) * 1000;
    select(0, NULL, NULL, NULL, &t);
#else
	Sleep(milli);
#endif
}

int SYS_GetMilli(void) {
#ifndef _WIN32
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
#else
	int curtime;
	static int		base;
	static bool	initialized = false;

	if (!initialized)
	{	// let base retain 16 bits of effectively random data
		base = timeGetTime() & 0xffff0000;
		initialized = true;
	}
	curtime = timeGetTime() - base;
	return curtime;
#endif
}

void SYS_UpdateTime(int minmilli) {
    static int LastTime = 0;
    if (LastTime == 0) {
        LastTime = SYS_GetMilli();
        SYS_Sleep(1000/60);
    }
    g_Time = SYS_GetMilli();
    g_FrameDelta = g_Time - LastTime;
    if (g_FrameDelta < minmilli) {
        SYS_Sleep(minmilli - g_FrameDelta);
        g_Time = SYS_GetMilli();
        g_FrameDelta = g_Time - LastTime;
    }
    LastTime = g_Time;
}

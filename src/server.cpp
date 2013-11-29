#include "defs.h"

void SV_Init()
{
    SV_Start();
}

void SV_Cleanup()
{
    MSG_t msg;
    while(SV_Read(&msg));
    SV_Free();
}

void SV_Run()
{
    MSG_t msg;

    while(SV_Read(&msg)) {
        if (msg.data[0] != MSG_SETPOS) {
            continue;
        }
        SV_BroadCast(&msg, msg.data[1]);
    }
}

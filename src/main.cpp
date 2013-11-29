// http://stackoverflow.com/questions/6422578/how-do-i-support-different-opengl-versions
#include <cstring>
#include "defs.h"

bool g_IsServer;
bool g_IsClient;
int g_ClientId;
const char *serverAddr;

static void readoptions(int argc, char **argv)
{
    g_IsServer = true;
    g_IsClient = true;
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-c") == 0) {
            serverAddr = argv[i + 1];
            g_IsServer = false;
            g_IsClient = true;
            i++;
        } else if (strcmp(argv[i], "-d") == 0) {
            g_IsServer = true;
            g_IsClient = false;
        }
    }
}

int main(int argc, char **argv)
{
    readoptions(argc, argv);
    if (g_IsServer) {
        SV_Init();
    }
    if (g_IsClient) {
        CL_Init(serverAddr);
    }
    SYS_UpdateTime();
    for (;;) {
        if (g_IsClient && !VID_IsRunning()) {
            break;
        }
        if (g_IsServer) {
            SV_Run();
        }
        if (g_IsClient) {
            if (!CL_Run()) {
                break;
            }
        } else {
            SYS_UpdateTime();
        }
    }
    if (g_IsClient) {
        CL_Cleanup();
    }
    if (g_IsServer) {
        SV_Cleanup();
    }
    return 0;
}

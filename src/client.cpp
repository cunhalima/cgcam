#include <cstring>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "defs.h"
#include "camera.h"

static int last_ClientId;

struct peer_t {
    glm::vec3 pos;
} peer[MAX_CLIENTS];

void CL_Input()
{
    MSG_t msg;

    if (!g_ClientId) {
        return;
    }
    glm::vec3& cl_pos = peer[g_ClientId - 1].pos;
    int cmd = VID_GetCmd();
    if (cmd & 1) {
        cl_pos.x += 0.7f * g_FrameDelta;
    }
    if (cmd & 2) {
        cl_pos.x -= 0.7f * g_FrameDelta;
    }
    msg.data[0] = MSG_SETPOS;
    msg.data[1] = g_ClientId;
    memcpy(&msg.data[4], &cl_pos[0], sizeof(cl_pos[0]) * 3);
    CL_Send(&msg);
}

void CL_RenderAll()
{
    VID_BeginFrame();
    glm::mat4 Projection;
    VID_GetMatrix(&Projection[0][0]);
    glm::mat4 View;
    glm::mat4 rot = glm::mat4(1);
    rot = glm::rotate(rot, g_CameraInterface.rot.x, glm::vec3(1, 0, 0));
    rot = glm::rotate(rot, g_CameraInterface.rot.y, glm::vec3(0, 1, 0));
    rot = glm::rotate(rot, g_CameraInterface.rot.z, glm::vec3(0, 0, 1));
    View = glm::transpose(rot);
    View = glm::translate(View, glm::vec3(-g_CameraInterface.pos.x, -g_CameraInterface.pos.y, -g_CameraInterface.pos.z));
    #if 0
    /*
    glm::mat4 View = glm::lookAt(
        //glm::vec3(0,0,0),
        g_CameraInterface.pos,
        glm::vec3(0,0,-1),
        glm::vec3(0,0,0),
        glm::vec3(0,0,-1),
        glm::vec3(0,1,0));
    */
    #endif
    //glm::mat4 Model = glm::mat4(1);
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (peer[i].pos.x <= -1000.0f) {
            continue;
        }
        glm::mat4 Model = glm::translate(glm::mat4(1), peer[i].pos);
        glm::mat4 MVP = Projection * View * Model;
        MAP_Render(&MVP[0][0]);
    }

    VID_EndFrame();
}

void CL_SpawnSelf()
{
    if (last_ClientId != g_ClientId) {
        last_ClientId = g_ClientId;
        if (g_ClientId > 0) {
            peer[g_ClientId - 1].pos = glm::vec3(-2.0f + 2.0f * g_ClientId, 0.0f, -5.0f);
        }
    }
}

bool CL_Run()
{
    MSG_t msg;

    if (!VID_IsRunning()) {
        return false;
    }
    while(CL_Read(&msg)) {
        if (msg.data[0] != MSG_SETPOS) {
            continue;
        }
        int i = msg.data[1] - 1;
        if (i >= 0 && i < MAX_CLIENTS) {
            memcpy(&peer[i].pos[0], &msg.data[4], sizeof(peer[i].pos[0]) * 3);
        }
    }
    CL_SpawnSelf();
    CL_Input();
    CL_RenderAll();
    return true;
}

void CL_Init(const char *ipaddr)
{
    g_ClientId = last_ClientId = 0;
    for (int i = 0; i < MAX_CLIENTS; i++) {
        peer[i].pos = glm::vec3(-1000.0f, -100.0f, 1.0f);
    }
    VID_Init(640, 400);
    VID_LoadTextures();
    VID_LoadShaders();
    MAP_Load();

    CL_Start();
    if (g_IsServer) {
        CL_Connect("127.0.0.1");
    } else {
        CL_Connect(ipaddr);
    }
}

void CL_Cleanup()
{
    MSG_t msg;

    MAP_Free();
    VID_FreeShaders();
    VID_FreeTextures();
    VID_Cleanup();
    while(CL_Read(&msg));
    CL_Free();
}

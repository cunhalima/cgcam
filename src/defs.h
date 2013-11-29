#ifndef INC_DEFS_H
#define INC_DEFS_H

#define TEX_BASIC0      0
#define SHADER_COLORTEX 0

enum {
    SHADERID_POS = 0,
    SHADERID_COLOR,
    SHADERID_UV,
    SHADERID_TEX,
    SHADERID_MAT,
    MAX_SHADERIDS,
};

#define MSG_SIZE    32
#define MSG_SETPOS  10
#define MAX_CLIENTS 8
class MSG_t {
public:
    int to;
    int from;
    int order;
    char data[MSG_SIZE];
};

extern double g_Time, g_FrameDelta;
extern bool g_IsServer, g_IsClient;
extern int g_ClientId;

unsigned int LoadShaders(const char * vertex_file_path, const char * fragment_file_path);

bool VID_IsRunning();
void VID_GetMatrix(float *m);
int VID_Init(int w, int h);
void VID_Cleanup();
void VID_BeginFrame();
void VID_EndFrame();
int VID_GetCmd();

void MODEL_Init();
void MODEL_Free();
void MODEL_Render();

void MAP_Load();
void MAP_Free();
void MAP_Render(const float *m);

void CL_Init(const char *ipaddr);
bool CL_Run();
void CL_Cleanup();

void SV_Init();
void SV_Run();
void SV_Cleanup();

void SYS_UpdateTime();

unsigned int loadBMP_custom(const char * imagepath);

void SV_Start();
void SV_Free();
void SV_Send(MSG_t *msg);
void SV_BroadCast(MSG_t *msg, int notto);
bool SV_Read(MSG_t *msg);
void CL_Start();
void CL_Free();
void CL_Connect(const char *ipaddr);
void CL_Disconnect();
void CL_Send(MSG_t *msg);
bool CL_Read(MSG_t *msg);

void VID_LoadTexture(int ti, const char *tn);
void VID_LoadTextures();
void VID_FreeTextures();
void VID_LoadShader(int si, const char *vs, const char *fs);
void VID_SetShaderId(int si, int sid, const char *name);
void VID_LoadShaders();
void VID_FreeShaders();
void VID_SetUniform(int sid, const void *m);
void VID_SetUniform(int sid, int n);
void VID_SelectShader(int si);
void VID_SelectTexture(int ui, int ti);
unsigned int VID_GetShaderId(int sid);

#endif

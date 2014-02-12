#ifndef INC_DEFS_H
#define INC_DEFS_H


#define INVALID_ENTITY_ID	-1
#define IS_VALID_ENTITY_ID(i) ((i) >= 0 && (i) < MAX_GAME_ENTITIES)
#define INVALID_CLIENT_ID	-1
#define IS_VALID_CLIENT_ID(i) ((i) >= 0 && (i) < MAX_CLIENTS)
#define INVALID_MODEL_ID	-1
#define IS_VALID_MODEL_ID(i) ((i) >= 0 && (i) < MAX_MODELS)


//================================================
// Global definitions
//================================================
#define SERVER_TIC  100

#define ANG_PITCH	0
#define ANG_YAW		1
#define ANG_ROLL	2

#include <cstddef>

#ifndef NULL
#define NULL 0
#endif

char *my_strdup(const char *original);

typedef float vec3_t[3];
typedef unsigned int gtime_t;

// important globals
extern int g_Time, g_FrameDelta;
extern bool g_QuitFlag;			// if true, we should quit
extern bool g_ConsoleOn;
void readoptions(int argc, char **argv);

struct Options_t {
	int xres, yres;				// desired resolution
	bool fullscreen;
	bool client;				// wish to run client
	bool server;				// wish to run server
	const char *remoteHost;
	int argc;
	char **argv;
	int winpos;
	bool nolog;
	bool forceGL21;
};

extern Options_t opt;

//================================================
// Common (for both Server and Client) (common)
//================================================
typedef enum {
    CS_DISCONNECTED = 0,
	CS_CONNECTING,
    CS_DISCONNECTING,
    CS_SPAWNED,
} constate_t;

const int MAX_GAME_ENTITIES = 16;		// Maximum number of entities in the game
const int MAX_SNAPSHOTS = 32;			// max snapshots stored in cyclic buffer
const int MAX_PACKET_SIZE = 2048;

// szb.cpp (sizeable buffer)
struct szb_t;
szb_t*  szb_create();
void	szb_free(szb_t *buf);
void	szb_write8(szb_t *buf, int v);
void	szb_write16(szb_t *buf, int v);
void	szb_write32(szb_t *buf, int v);
void	szb_writeAng8(szb_t *buf, float ang);
void	szb_writeAng16(szb_t *buf, float ang);
void	szb_writeCoord(szb_t *buf, float coord);
void	szb_writePos(szb_t *buf, const float *xyz);
int		szb_read8(szb_t *buf);
int		szb_read16(szb_t *buf);
int		szb_read32(szb_t *buf);
float	szb_readAng8(szb_t *buf);
float	szb_readAng16(szb_t *buf);
float	szb_readCoord(szb_t *buf);
void	szb_readPos(szb_t *buf, float *xyz);
void	szb_print(szb_t *buf);
void	szb_rewind(szb_t *buf);
void	szb_test();
void	szb_resize(szb_t *buf, int size);
void*	szb_getPtr(szb_t *buf);
int 	szb_getSize(szb_t *buf);
bool	szb_eof(szb_t *buf);

// assets.cpp
const char *ASS_GetModelPath(const char *modelname, const char *filename);
const char *ASS_GetScriptPath(const char *filename);
const char *ASS_GetShaderPath(const char *filename);
const char *ASS_GetPath(const char *filename);

// console.cpp
#if defined(_MSC_VER)
#	define FUNC_NAME	__FUNCTION__
#else
#	define FUNC_NAME	__func__
#endif
typedef void (*conprintfunc_t)(const char *text);
#define CON_OK()      con_printf("%-32s: [OK]\n", FUNC_NAME)
#define CON_ERR(s)    con_printf("%-32s: [FAILED]: %s\n", FUNC_NAME, s)
#define CON_ERRS(a,b) con_printf("%-32s: [FAILED]: " a "\n", FUNC_NAME, b)
void con_init();
void con_cleanup();
void con_logprint(const char *msg);
void con_print(const char *msg);
void con_printf(const char *fmt, ...);
void con_redirectOutput(conprintfunc_t func);

// cvar.cpp
struct cvar_t {
	char *str;
	float number;
	int integer;
};
void cvar_set(cvar_t *cvar, const char *str);
void cvar_set(cvar_t *cvar, float number);
void cvar_set(cvar_t *cvar, int integer);
cvar_t *cvar_get(const char *name);
cvar_t *cvar_register(const char *name, const char *v);

// cfun.cpp
typedef void (*cfun_t)(void);
bool cfun_register(cfun_t func, const char *name);
int cfun_argc();
int cfun_argi(int i);
bool cfun_argdown();
const char *cfun_args(int i);
void con_type(const char *text);
void con_run();
void con_addlines(const char *lines);

// net.cpp
extern cvar_t *var_dbconn;
struct nnode_t;
#define NS_CLIENT       0
#define NS_SERVER       1
nnode_t *nnode_make();
void nnode_free(nnode_t *nn);
nnode_t *nnode_dup(nnode_t *nn);
int nnode_cmp(nnode_t *a, nnode_t *b);
bool nnode_setHost(nnode_t *nn, const char *host);
void net_init();
void net_cleanup();
bool net_startServer();
bool net_stopServer();
bool net_startClient();
bool net_stopClient();
bool net_send(int ns, nnode_t *nn, szb_t *msg);
bool net_recv(int ns, nnode_t **nn, szb_t *msg);
void net_test();

// math.cpp
void M_VectorCopy(vec3_t dst, const vec3_t src);

//================================================
// Server
//================================================
const int MAX_CLIENTS = 8;
enum {
	C2S_CONNECT = 0,
	C2S_CONNLESS,
	C2S_DISCONNECT,
	C2S_STATE,
};
enum {
	S2C_SNAPSHOT = 0,
};

struct svEntity_t {
    bool used;			// this slot is in use (entity exists) or not
	int id;			// starts with 0 (-1 is invalid entity)
	vec3_t origin;
	vec3_t angles;
    int frame;			// current frame number
    int model;			// current model
};

struct svPlayer_t {
	int lastFrame;
	bool moving;
	bool attack;
	bool jump;

	bool pain;

	bool doAttack;
	vec3_t target;
	svEntity_t *targetEntity;
};

struct svSnapshot_t {
	int numEntities;				// number of entities sent
	bool sent[MAX_GAME_ENTITIES];			//
	svEntity_t entities[MAX_GAME_ENTITIES];
};

// The client, from the server's point of view
struct svClient_t {
    int id;				// starts with 0 (-1 is invalid client)
    constate_t constate;
	int lastReceivedFrame;			// the number of the last received frame (starts with 0, -1 = invalid frame)
	svSnapshot_t snapshots[MAX_SNAPSHOTS];
    nnode_t *nn;					// network node
	int lastPacketTime;				// last time we got a packet from this client (for disconnection purposes)
};

// The server
struct svState_t {
	int time;						// the current time of the server in milliseconsds (increments of 100)
	int frame;						// the current frame of the server (starts with 0, -1 = invalid frame)
	svClient_t clients[MAX_CLIENTS];// the clients from the server point of view
	svPlayer_t players[MAX_CLIENTS];
	svEntity_t entities[MAX_GAME_ENTITIES];
	bool online;					// the server is running or not
	bool initialized;					// server stuff has been allocated
};

extern svState_t ss;

// sv_main.cpp
void SV_Init();
void SV_Cleanup();
bool SV_Start();
bool SV_Stop();
void SV_Frame();
//void SV_test();
svEntity_t *SV_SpawnEntity(int i);
bool SV_RemoveEntity(int i);

// sv_connect.cpp
int SV_FindClientByNNode(nnode_t *nn);
int SV_AddClientWithNNode(nnode_t *nn);
svClient_t *SV_GetClientById(int id);
void SV_ConnAttempt(nnode_t *nn, szb_t *msg);
void SV_Connless(nnode_t *nn, szb_t *msg);
void SV_SilentDrop(svClient_t *c);

// sv_entity.cpp
svEntity_t *SV_GetEntityAt(vec3_t pos);
svEntity_t *SV_SpawnEntity(int i);
int SV_SpawnEntity();
bool SV_RemoveEntity(int i);
svEntity_t *SV_GetEntityById(int id);
void SV_WriteEntity(szb_t *msg, svEntity_t *e);

// sv_snapshot.cpp
void SV_BuildSnapshot(szb_t *msg, svClient_t *c);

// sv_player.cpp
void SV_HitEntity(svEntity_t *e);
void SV_Think(svEntity_t *e);
svPlayer_t *SV_GetPlayerFromEntity(svEntity_t *e);

// sv_clientstate.cpp
void SV_ReadClientState(svClient_t *c, szb_t *msg);


//================================================
// Client
//================================================
extern cvar_t *var_cl_connattempts;

struct clEntity_t {
    bool used;			// this slot is in use (entity exists) or not
	int id;				// starts with 0 (-1 is invalid entity)
	vec3_t origin;
	vec3_t angles;
    int frame;			// current frame number
    int model;			// current model
};

struct clSnapshot_t {
	//int numEntities;
	bool valid;
	int frame;
	int time;
	clEntity_t entities[MAX_GAME_ENTITIES];
};

// The server, from the client's point of view
struct clServer_t {
    constate_t constate;
	int connAttempts;
	char *host;
	clSnapshot_t snapshots[MAX_SNAPSHOTS];
	nnode_t *nn;
	int currTime;				// server time in latest received snapshot (in the past)
	int currFrame;				// latest frame (in the past)
	int probableTime;			// probable current server time (based on statistics)
	int disconCountdown;
    //int servertime;
    //int pservertime;
};

struct clPlayer_t {
	clEntity_t entity;
	bool forward;
	bool back;
	bool left;
	bool right;
	bool jump;
	bool attack;

	bool moving;

	int attackDelay;
	bool sendAttack;
};

// The client
struct clState_t {
	ptrdiff_t lastSnapshotNumber;	// number of the last received snapshot
    int simTime;					// simulation time (will be used for interpolation)
    int realTime;					// real time is never adjusted
    int prevFrame;					// previous received server frame
	bool online;
	bool spawned;
	clPlayer_t player;
	clServer_t server;
	clSnapshot_t snapshots[3];		// only two snapshots for interpolation
	clSnapshot_t *snapA;			// current snapshot (0 or 1)
	clSnapshot_t *snapB;
	clSnapshot_t *snapC;
	int mouseX, mouseY, mouseXprev, mouseYprev;
	vec3_t viewAngles;
	vec3_t viewOrigin;
};

extern clState_t cs;

// cl_main.cpp
void CL_Init();
void CL_Cleanup();
bool CL_Connect(const char *host);
bool CL_Disconnect();
void CL_Frame();
bool CL_SilentDisconnect();

// cl_input.cpp
void CL_InputInit();
void CL_InputCleanup();
void CL_InputEvent(int key, bool down);
void CL_InputMouse(int x, int y);

// cl_render.cpp
void CL_RenderFrame();

// cl_player.cpp
void CL_PlayerInit();
void CL_PlayerCleanup();
void CL_PlayerMove();

// cl_connect.cpp

void CL_ForceDisconnect();
bool CL_Connect(const char *host);
bool CL_SilentDisconnect();
void CL_SendDisconnectPacket();
bool CL_Disconnect();
void CL_ConnEstablished();
void CL_ConnectAttempt();

// cl_game.cpp
void CL_GameInit();
void CL_GameCleanup();

//================================================
// Sound
//================================================

// snd_main.cpp
bool SND_Init();
void SND_Cleanup();

//================================================
// System (low level) stuff (sys)
//================================================

// sys_glfw.cpp
void SYS_FlipScreenPage();
void SYS_CloseVideo();
bool SYS_InitVideo();
void SYS_Init();
void SYS_Cleanup();
bool SYS_IsRunning();
void SYS_PollEvents();
int SYS_GetKeyNumber(const char *name);
int SYS_GetNumKeys();
void SYS_GetInitialMousePos(int *x, int *y);

// timer.cpp
void SYS_Sleep(int milli);
void SYS_UpdateTime(int minmilli = 0);

//================================================
// Platform-dependent stuff (Unix, Windows etc)
//================================================

// conwin.cpp, conunix.cpp
bool Syscon_Init();
void Syscon_ProcessEvents();
void Syscon_Cleanup();
void Syscon_Print(const char *txt);
void SYS_ShowError(const char *txt);
void SYS_ReadOptions(int argc, char **argv);

//================================================
// Refresher (ref)
//================================================
const int MAX_MODELS = 16;
const int MAX_DRAW_ENTITIES = 16;
const int MAX_DRAW_PARTICLES = 32;

struct refEntity_t {
	vec3_t origin;
	vec3_t angles;
	int model;
	int frame1, frame2;
	refEntity_t *next;
};

struct refParticle_t {
	vec3_t origin;
	vec3_t velocity;
	int type;
	int deathtime;
	refParticle_t *next;
};

struct refState_t {
	int xres, yres;
	bool on;
	bool coreGL;
	vec3_t targetOrigin;
	refEntity_t *models[MAX_MODELS];
	refEntity_t drawEnts[MAX_DRAW_ENTITIES];
	refParticle_t particles[MAX_DRAW_PARTICLES];
	int numDrawEntities;
};

class map_t;

extern refState_t rs;

typedef unsigned int glid_t;		// to prevent using GLuint directly

const int TEX_NN = 1;				// nearest neighbor
const int TEX_LM = 2;				// linear
const int TEX_CLAMP = 4;			// clamp to edge
const int TEX_REPEAT = 8;			// repeat texture
const int TEX_MIP = 16;				// mipmapping

// re_main.cpp
float RE_GetCenterZ();
void RE_ClearBuffers();
void RE_ResetState();
void RE_PrintGLErrors(int line, const char *filename = NULL);
#define GL_DEBUG()	RE_PrintGLErrors(__LINE__, __FILE__)

// re_shader.cpp
glid_t RE_LoadShaders(const char* vert, const char* frag);

// re_texture.cpp
glid_t RE_LoadTexture(const char *path, int flags);

// re_text.cpp
void RE_2DInit();
void RE_2DClear();
void RE_2DSetColor(float r, float g, float b);
void RE_2DCleanup();
void RE_2DRender();
void RE_2DRect(int x1, int y1, int x2, int y2);
void RE_TextPrint(int x, int y, const char *text);
void RE_TextRender();
void RE_TextClear();

// re_vidcon.cpp
void RE_VidconInit();
void RE_VidconCleanup();
void RE_VidconDraw();
void RE_VidconResize();
void RE_VidconKeyPress(int ch);
void RE_VidconCharPress(int ch);

// re_md2.cpp
#if 0
int MD2_Load(const char *name);
void MD2_Render(int model, const float *m, int first, int last);
void MD2_SetupTexture(int model);
void MD2_BeginRender(float amount);
void MD2_EndRender();
void MD2_Init();
void MD2_Cleanup();
int MD2_Get(const char *name);
#endif

class MDL_t;

MDL_t *MDL_Load(const char *name, float scale = 1.0f);
void MDL_Render(MDL_t *mdl, int first, int last);
void MDL_PrepareTexture(MDL_t *mdl);
void MDL_BeginRender(int shader, float amount);
void MDL_EndRender();
void MDL_Init();
void MDL_Cleanup();
int MDL_Get(const char *name);
MDL_t* MDL_Get(int id);
void MDL_SetModelMatrix(const float *m);
void MDL_SetViewMatrix(const float *m);
void MDL_SetMVPMatrix(const float *m);
void MDL_SetLightPos(const float *p);
void MDL_SetLightParams(float attenuation, float ambient);

// re_scene.cpp
void RE_AddRefEntityToScene(refEntity_t *original);
void RE_BeginScene();
void RE_EndScene();
void RE_RenderScene(float lerp);

// re_map.cpp
extern map_t *thismap;

map_t *MAP_Load(const char *path);
void MAP_Free(map_t *pmap);
void MAP_Render(map_t *pmap);
void MAP_Init();
void MAP_Cleanup();
void MAP_SetModelMatrix(const float *m);
void MAP_SetViewMatrix(const float *m);
void MAP_SetMVPMatrix(const float *m);
void MAP_BeginRender();

//re_particle.cpp
void RE_AddParticleToScene(refParticle_t *original);
void RE_RenderParticles(const float *matV, const float *matP);
void RE_ParticlesInit();
void RE_ParticlesCleanup();
void RE_AddSplash(vec3_t origin);


//================================================
// Tests etc
//================================================

// re_test1.cpp
void T01_Init();
void T01_Frame();
void T01_Cleanup();

// re_t03.cpp
void T03_Init();
void T03_Frame();
void T03_Cleanup();

#endif
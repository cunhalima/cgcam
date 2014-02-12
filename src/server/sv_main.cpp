#include <cassert>
#include <cstring>
#include "../defs.h"

svState_t ss;

static void start_f();
static void stop_f();
static void servertest_f();

static cvar_t *var_sv_tic;
static cvar_t *var_sv_droptimeout;

void SV_Init() {
	memset(&ss, 0, sizeof(ss));
	cfun_register(start_f, "start");
	cfun_register(stop_f, "stop");
	cfun_register(servertest_f, "servertest");
	var_sv_tic = cvar_register("sv_tic", "1000");
	var_sv_droptimeout = cvar_register("sv_droptimeout", "3000");
	cvar_set(var_sv_tic, SERVER_TIC);
	ss.initialized = true;
    CON_OK();
}

void SV_Cleanup() {
	if (!ss.initialized) {
		return;
	}
	if (ss.online) {
        SV_Stop();
    }
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (ss.clients[i].nn != NULL) {
            nnode_free(ss.clients[i].nn);
            ss.clients[i].nn = NULL;
        }
    }
	memset(&ss, 0, sizeof(ss));
}

//==============================================================

//static int lua_sv_start(lua_State *L)
//{
//    LUA_ARGS(0);
//    sv_start();
//    return 0;
//}
#if 0
static int lua_SV_SpawnEntity(lua_State *L)
{
    LUA_ARGS(0);
    lua_pushinteger(L, SV_SpawnEntity());
    return 1;
}

static int lua_ent_remove(lua_State *L)
{
    LUA_ARGS(1);
    lua_pushboolean(L, ent_remove(lua_tointeger(L, 1)));
    return 1;
}

static const luaL_Reg funcs[] = {
    {"sv_start", lua_sv_start},
    {"SV_SpawnEntity", lua_SV_SpawnEntity},
    {"ent_remove", lua_ent_remove},
    {NULL, NULL}
};
#endif

bool SV_Start() {
	if (ss.online) {
        CON_ERR("server already started");
        return false;
    }
    net_startServer();
    ss.online = true;
    return true;
}

bool SV_Stop() {
	if (!ss.online) {
        CON_ERR("server not started");
        return false;
    }
    net_stopServer();
	ss.online = false;
    return true;
}

bool SV_TimeToThink() {
    static int nextupdate = 0;

	if (g_Time < nextupdate) {
        return false;
    }
	int tic = var_sv_tic->integer;
    nextupdate += tic;
    if (g_Time >= nextupdate) {
        nextupdate = g_Time + tic;
    }
    return true;
}

void SV_ReceivePackets() {
    nnode_t *nn;

	szb_t *msg = szb_create();
	for (;;) {
	    szb_resize(msg, MAX_PACKET_SIZE);
		if (net_recv(NS_SERVER, &nn, msg) == 0) {
			break;
		}
		//con_printf("olha aqui:\n");
		//szb_print(msg);
        int cmd = szb_read8(msg);
        if (cmd == C2S_CONNLESS) {
            SV_Connless(nn, msg);
            continue;
        }
		int i = SV_FindClientByNNode(nn);
		if (i == INVALID_CLIENT_ID) {
			if (var_dbconn->integer != 0) {
				CON_ERR("got conn packet from someone not connected"); // Should ignore?
			}
			continue;
		}

		szb_rewind(msg);
		svClient_t *c = SV_GetClientById(i);
		c->lastPacketTime = g_Time;
        while (!szb_eof(msg)) {
            int cmd = szb_read8(msg);
            switch(cmd) {
                case C2S_STATE:
                    SV_ReadClientState(c, msg);
                    break;
                default:
                    CON_ERR("invalid cmd");
                    break;
            }
        }
		

        #if 0
        szb_rewind(packdata);
        while (!szb_eof(packdata)) {
            int cmd = szb_read8();
            switch(cmd) {
                case C2S_STATE:
                    sv_readplayerstate(packdata);
                    break;
                default:
                    con_printf(__func__ ": unknown cmd: %d\n", cmd);
                    break;
            }
        }
        #endif
    }
	szb_free(msg);
}

// Runs 10 times/second
void SV_GameFrame() {
	/*
    lua_State *L = g_L;
    lua_getglobal(L, "sv_frame");
    if (lua_pcall(L, 0, 0, 0) != 0) {
        const char *str = lua_tostring(L, -1);
        CON_ERR(str);
        lua_pop(L, 1);
    }
	*/
    //con_printf("Running %s\n", __func__);
    #if 0
    for (int i = 0; i < MAX_GAME_ENTITIES; i++) {
        entity_t *e = &ss.entities[i];
        if (!e->used) {
            continue;
        }
        sv_move(e);
        sv_anim(e);        
    }
    #endif
    for (int i = 0; i < MAX_GAME_ENTITIES; i++) {
		svEntity_t *e = SV_GetEntityById(i);
        if (!e->used) {
            continue;
        }
        SV_Think(e);
    }
}

void SV_SendPacket(szb_t *msg, svClient_t *c) {
	assert(msg != NULL);
	assert(c != NULL);
    net_send(NS_SERVER, c->nn, msg);
	if (var_dbconn->integer != 0) {
        CON_OK();
    }
}

void SV_AddAttacks(szb_t *msg, svClient_t *c) {
	assert(msg != NULL);
	assert(c != NULL);
    for (int i = 0; i < MAX_CLIENTS; i++) {
		if (ss.players[i].doAttack) {
			szb_write8(msg, i);
			if (ss.players[i].targetEntity == NULL) {
				szb_write8(msg, INVALID_ENTITY_ID);
			} else {
				szb_write8(msg, ss.players[i].targetEntity->id);
			}
			szb_writePos(msg, ss.players[i].target);
		}
	}
	szb_write8(msg, -1);
}

void SV_ClearAttacks() {
    for (int i = 0; i < MAX_CLIENTS; i++) {
		ss.players[i].doAttack = false;
	}
}

void SV_SendPackets() {
	assert(ss.online);
	szb_t *msg = szb_create();
    szb_resize(msg, MAX_PACKET_SIZE);
    for (int i = 0; i < MAX_CLIENTS; i++) {
		svClient_t *c = SV_GetClientById(i);
        if (c->constate != CS_SPAWNED) {
            continue;
        }
		if (c->lastPacketTime + var_sv_droptimeout->integer < g_Time) {
			SV_SilentDrop(c);
			continue;
		}
        szb_resize(msg, 0);
		szb_write8(msg, c->id);
	    szb_write32(msg, ss.frame);
        SV_BuildSnapshot(msg, c);
		SV_AddAttacks(msg, c);
        SV_SendPacket(msg, c);
    }
	SV_ClearAttacks();
	szb_free(msg);
}

void SV_Frame() {
    if (!ss.online) {
        return;
    }
    SV_ReceivePackets();
    // Proceed only 10 times per second
    if (!SV_TimeToThink()) {
        return;
    }
    ss.frame++;
    SV_GameFrame();
    #if 0
    // Pause should be checked here in order to keep reading clients' msgs
    if (cvar_bool(cvar_paused)) {
        return;
    }
    #endif
    SV_SendPackets();
}

#if 0
void luag_getVec3(float *v)
{
    // expectes a table in -1
    for (int i = 1; i <= 3; i++) {
        lua_pushinteger(g_L, i);
        lua_gettable(g_L, -2);
        if (lua_isnumber(g_L, -1)) {
            v[i - 1] = lua_tonumber(g_L, -1);
        }
        lua_pop(g_L, 1);
    }
}

void luag_getEntity(svEntity_t *e)
{
    e->used = true;
    // expectes a table in -1
    lua_getfield(g_L, -1, "pos");
    if (lua_istable(g_L, -1)) {
        luag_getVec3(e->pos);
        //con_printf("::  %f %f %f\n", e->pos[0], e->pos[1], e->pos[2]);
    }
    lua_pop(g_L, 1);
    lua_getfield(g_L, -1, "ang");
    if (lua_istable(g_L, -1)) {
        luag_getVec3(e->ang);
        //con_printf("::  %f %f %f\n", e->ang[0], e->ang[1], e->ang[2]);
    }
    lua_pop(g_L, 1);
    lua_getfield(g_L, -1, "frame");
    if (lua_isnumber(g_L, -1)) {
        e->frame = lua_tointeger(g_L, -1);
        //con_printf("::  %d\n", e->frame);
    }
    lua_pop(g_L, 1);
    lua_getfield(g_L, -1, "model");
    if (lua_isnumber(g_L, -1)) {
        e->model = lua_tointeger(g_L, -1);
        //con_printf("::  %d\n", e->model);
    }
    lua_pop(g_L, 1);
}


void list_entities(lua_State *L)
{
    for (int i = 0; i < MAX_GAME_ENTITIES; i++) {
        ss.entities[i].used = false;
    }
    lua_getglobal(L, "entities");
    if (lua_istable(L, -1)) {
        //con_print("<entities>\n");
        // stack now contains: -1 => table
        lua_pushnil(L);
        // stack now contains: -1 => nil; -2 => table
        while (lua_next(L, -2))
        {
            // stack now contains: -1 => value; -2 => key; -3 => table
            lua_pushvalue(L, -2);
            int id = 0;
            // stack now contains: -1 -=> key; -2 => value; -3 => key; -4 => table
            if (lua_isnumber(L, -1)) {
                id = lua_tointeger(L, -1);
            }
            lua_pop(L, 1);
            // stack now contains: -1 => value; -2 => key; -3 => table
            //con_stackDump();
            if (unsigned(id - 1) < MAX_GAME_ENTITIES) {
                luag_getEntity(&ss.entities[id - 1]);
            }
            lua_pop(L, 1);
            // stack now contains: -1 => key; -2 => table
        }
        // stack now contains: -1 => table
    }
    lua_pop(L, 1);
}

void sv_test()
{
    lua_State *L = g_L;
    con_print("aaaaee\n");
    list_entities(L);
}

#endif

//==================================================================









//void CL_MouseButton

// ===== console functions ======
void start_f() {
	con_printf("starting server\n");
	SV_Start();
}

void stop_f() {
	con_printf("stopping server\n");
	SV_Stop();
}

void servertest_f() {
	con_printf("testing server\n");

	szb_t* msg = szb_create();

	//----------------
	int i = -1;
	con_printf("writing value %d\n", i);
	szb_rewind(msg);
	szb_write16(msg, -1);
	szb_rewind(msg);
	i = szb_read16(msg);
	con_printf("reading value %d\n", i);
	//----------------
	i = -1;
	con_printf("writing value %d\n", i);
	szb_rewind(msg);
	szb_write8(msg, -1);
	szb_rewind(msg);
	i = szb_read8(msg);
	con_printf("reading value %d\n", i);
	//----------------
	float ang = 1.0f;
	con_printf("writing ang %f\n", ang);
	szb_rewind(msg);
	szb_writeAng8(msg, ang);
	szb_rewind(msg);
	ang = szb_readAng8(msg);
	con_printf("reading ang %f\n", ang);
	//----------------
	ang = 357.0f;
	con_printf("writing ang %f\n", ang);
	szb_rewind(msg);
	szb_writeAng8(msg, ang);
	szb_rewind(msg);
	ang = szb_readAng8(msg);
	con_printf("reading ang %f\n", ang);
	//----------------
	ang = -10.0f;
	con_printf("writing ang %f\n", ang);
	szb_rewind(msg);
	szb_writeAng8(msg, ang);
	szb_rewind(msg);
	ang = szb_readAng8(msg);
	con_printf("reading ang %f\n", ang);
	//----------------
	ang = 370.0f;
	con_printf("writing ang %f\n", ang);
	szb_rewind(msg);
	szb_writeAng8(msg, ang);
	szb_rewind(msg);
	ang = szb_readAng8(msg);
	con_printf("reading ang %f\n", ang);
	//----------------
	ang = 7260.0f;
	con_printf("writing ang %f\n", ang);
	szb_rewind(msg);
	szb_writeAng8(msg, ang);
	szb_rewind(msg);
	ang = szb_readAng8(msg);
	con_printf("reading ang %f\n", ang);
	//----------------
	ang = 280.0f;
	con_printf("writing ang %f\n", ang);
	szb_rewind(msg);
	szb_writeAng8(msg, ang);
	szb_rewind(msg);
	ang = szb_readAng8(msg);
	con_printf("reading ang %f\n", ang);
	//----------------

	szb_free(msg);
}

// =====

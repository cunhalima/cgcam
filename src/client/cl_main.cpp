#include <string>
#include <cstring>
#include <cassert>
#include <cstdlib>
#include "../defs.h"
//#include "luacon.h"

clState_t cs;

static void connect_f();
static void disconnect_f();

static cvar_t *var_cl_tic;
static cvar_t *var_cl_timeshift;
cvar_t *var_cl_connattempts;

void CL_Init() {
	CL_InputInit();
	CL_PlayerInit();
	memset(&cs, 0, sizeof(cs));
	cs.lastSnapshotNumber = -1;	// last does not exist
	cfun_register(connect_f, "connect");
	cfun_register(disconnect_f, "disconnect");
	var_cl_connattempts = cvar_register("cl_connattempts", "50");
	var_cl_tic = cvar_register("cl_tic", "50");
	var_cl_timeshift = cvar_register("cl_timeshift", "0");
	cvar_set(var_cl_tic, SERVER_TIC/2);
    CON_OK();
}

void CL_Cleanup() {
    if (cs.online) {
		CL_ForceDisconnect();
    }
	if (cs.server.host != NULL) {
		cs.server.host = NULL;
	}
	memset(&cs, 0, sizeof(cs));
	CL_PlayerCleanup();
	CL_InputCleanup();
}

void CL_CalcTime() {
	// we've just got a msg from server
    // calculate the probable server time
	// ideal situation when we get this msg: 
	//
	//                  <--------- one tic ------->
	//                                          <->  xpdelay
	//
	//                  [-----------------------C-][---------------------------][--
	//-----------------][-------------------------][---------------------------][--S

	//int id = cs.
	int xpdelay = SERVER_TIC * 4 / 50;
	int ideal = cs.server.currTime - SERVER_TIC - xpdelay + var_cl_timeshift->integer;
	int diff = ideal - cs.simTime;

	//con_printf("diff = %d\n", diff);

	if (diff < -SERVER_TIC || diff > SERVER_TIC) {
		// snap
		cs.simTime = ideal;
	} else {
		cs.simTime += diff / 4;
	}



#if 0
    int diff = cs.servertime - cs.pservertime;
    cs.pservertime += diff * 2 / 3;
    if (diff > (SERVER_TIC/2) || diff < -(SERVER_TIC/2)) {
        cs.pservertime = cs.servertime;
    }
    int iclienttime = cs.pservertime - SERVER_TIC - (SERVER_TIC/20);
    diff = iclienttime - cs.time;
    int maxtime = cs.servertime - SERVER_TIC;
    int mintime = maxtime - (SERVER_TIC/3);
    if (cs.time > maxtime) {
        con_printf("high clamp:: client = %d    server = %d\n", cs.time, cs.servertime);
        //cs.time = maxtime;
        cs.time += diff / 2;
    } else if (cs.time < mintime) {
        con_printf("low clamp::  client = %d    server = %d\n", cs.time, cs.servertime);
        //cs.time = mintime;
        cs.time += diff / 2;
    } else {
        cs.time += diff / 4;
    }
#endif
}

bool CL_TimeToThink() {
    static int nextupdate = 0;
    static int last_time = 0;

	if (cs.server.constate == CS_SPAWNED) {
        cs.simTime += g_Time - last_time;
        //con_printf("cl.time = %d\n", cs.time);
    }
    cs.realTime += g_Time - last_time;
    last_time = g_Time;
    if (g_Time < nextupdate) {
        return false;
    }
	int tic = var_cl_tic->integer;
    nextupdate += tic;
    if (g_Time >= nextupdate) {
        nextupdate = g_Time + tic;
    }
    return true;
}

void CL_Spawn() {
	cs.spawned = true;
	int id = cs.player.entity.id;
	memset(&cs.player, 0, sizeof(cs.player));
	clEntity_t *e = &cs.player.entity;

	e->id = id;
	e->model = id;
	e->origin[0] = 2.0f * float(id) - 1.0f;
	e->origin[2] = 2.375f;
	
}

clEntity_t *CL_GetSnapEntityWithId(clSnapshot_t *snap, int id) {
	assert(snap != NULL);
	if (id < 0 || id >= MAX_GAME_ENTITIES) {
		return NULL;
	}
    return &snap->entities[id];
}

void CL_ReadSnapshot(szb_t *msg) {
	assert(msg != NULL);
	clSnapshot_t* snap;

	if (cs.snapC == NULL) {
		cs.snapC = &cs.snapshots[0];
		snap = cs.snapC;
	} else if (cs.snapB == NULL) {
		cs.snapB = &cs.snapshots[0];
		cs.snapC = &cs.snapshots[1];
		snap = cs.snapC;
	} else if (cs.snapA == NULL) {
		cs.snapA = &cs.snapshots[0];
		cs.snapB = &cs.snapshots[1];
		cs.snapC = &cs.snapshots[2];
		snap = cs.snapC;
	} else {
		if (cs.snapA == &cs.snapshots[0]) {
			cs.snapA = &cs.snapshots[1];
			cs.snapB = &cs.snapshots[2];
			cs.snapC = &cs.snapshots[0];
		} else if (cs.snapA == &cs.snapshots[1]) {
			cs.snapA = &cs.snapshots[2];
			cs.snapB = &cs.snapshots[0];
			cs.snapC = &cs.snapshots[1];
		} else {
			cs.snapA = &cs.snapshots[0];
			cs.snapB = &cs.snapshots[1];
			cs.snapC = &cs.snapshots[2];
		}
		snap = cs.snapC;
	}
	snap->valid = true;
	snap->frame = cs.server.currFrame;
	snap->time = cs.server.currTime;
    for (int i = 0; i < MAX_GAME_ENTITIES; i++) {
		snap->entities[i].used = false;
    }
    int count = 0;
    for (;;) {
        int id = szb_read16(msg);
        if (id == INVALID_ENTITY_ID) {
            break;
        }
        count++;
		clEntity_t *e = CL_GetSnapEntityWithId(snap, id);
		e->used = true;
		e->id = id;
        e->angles[0] = szb_readAng8(msg);
        e->angles[1] = szb_readAng8(msg);
        e->angles[2] = szb_readAng8(msg);
        szb_readPos(msg, e->origin);
        e->frame = szb_read16(msg);
        e->model = szb_read16(msg);
    }
	if (var_dbconn->integer != 0) {
		con_printf("got %d ents\n", count);
	}
}

void CL_ReadAttacks(szb_t *msg) {
	assert(msg != NULL);
	while(szb_read8(msg) != -1) {
		vec3_t pos;
		int tid = szb_read8(msg);
		szb_readPos(msg, pos);
		con_printf("read atk %d\n", tid);
		RE_AddSplash(pos);
	}
}

void CL_ReadState(szb_t *msg) {
	assert(msg != NULL);
    CL_CalcTime();
    CL_ReadSnapshot(msg);
	CL_ReadAttacks(msg);
	if (var_dbconn->integer != 0) {
		con_printf("server time = %d\n", cs.server.currTime);
    }
}

void CL_ReceivePackets() {
    nnode_t *nn;
	szb_t *msg = szb_create();

	for (;;) {
	    szb_resize(msg, MAX_PACKET_SIZE);
		if (net_recv(NS_CLIENT, &nn, msg) == 0) {
			break;
		}
        if (var_dbconn->integer != 0) {
            con_printf("got stuff from server\n");
        }
		if (cs.server.constate == CS_CONNECTING) {
            CL_ConnEstablished();
        }
		if (cs.server.constate == CS_DISCONNECTING) {
			// ignore server packets while disconnecting
			continue;
		}
		cs.player.entity.id = szb_read8(msg);
		if (cs.server.constate == CS_SPAWNED && !cs.spawned) {
			// now that we have the id we can spawn
			CL_Spawn();
		}
		int frame = szb_read32(msg);
		if (frame < cs.server.currFrame) {
			// drop old packets
			continue;
		}
		cs.server.currFrame = frame;
		cs.server.currTime = cs.server.currFrame * SERVER_TIC;
        while (!szb_eof(msg)) {
            int cmd = szb_read8(msg);
            switch(cmd) {
                case S2C_SNAPSHOT:
                    CL_ReadState(msg);
                    break;
                default:
                    CON_ERR("invalid cmd");
                    break;
            }
        }
    }
	szb_free(msg);
}

void CL_SendState() {
	szb_t *msg = szb_create();
    szb_write8(msg, C2S_STATE);

	clEntity_t *e = &cs.player.entity;
    szb_writeAng8(msg, e->angles[0]);
    szb_writeAng8(msg, e->angles[1]);
    szb_writeAng8(msg, e->angles[2]);
    szb_writePos(msg, e->origin);
    szb_write16(msg, e->model);
    szb_write8(msg, cs.player.moving);
	szb_write8(msg, cs.player.attack);
	szb_write8(msg, cs.player.jump);

	szb_write8(msg, cs.player.sendAttack);
	if (cs.player.sendAttack) {
		cs.player.sendAttack = false;
		szb_writePos(msg, rs.targetOrigin);
	}

    net_send(NS_CLIENT, cs.server.nn, msg);

	szb_free(msg);
}

void CL_Frame() {
    if (cs.online) {
		//if (var_dbconn->integer != 0) {
		//	con_printf("###### client = %d    server = %d\n", cs.simTime, cs.server.currTime);
		//}
	    CL_ReceivePackets();
    }
	CL_PlayerMove();
	CL_RenderFrame();
    if (!cs.online) {
		return;
	}
    // Proceed only 10 times per second
    if (!CL_TimeToThink()) {
        return;
    }
	switch(cs.server.constate) {
        case CS_CONNECTING:
            CL_ConnectAttempt();
            break;
        case CS_DISCONNECTED:
            break;
        case CS_DISCONNECTING:
			CL_SendDisconnectPacket();
            break;
        case CS_SPAWNED:
			CL_SendState();
            break;
        default:
            CON_ERR("invalid constate");
            break;
    }
}

void connect_f() {
	int argc = cfun_argc();
	if (argc == 1) {
		CL_Connect("127.0.0.1");
	} else if (argc == 2) {
		CL_Connect(cfun_args(1));
	} else {
		con_print("Usage: connect <host>\n");
	}
}

void disconnect_f() {
	CL_Disconnect();
}

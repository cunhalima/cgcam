#include <cassert>
#include <cstring>
#include "../defs.h"

int SV_FindClientByNNode(nnode_t *nn) {
    assert(nn != NULL);
    for (int i = 0; i < MAX_CLIENTS; i++) {
        nnode_t *cnn = ss.clients[i].nn;
        if (cnn == NULL) {
            continue;
        }
        if (nnode_cmp(nn, cnn) == 0) {
            return i;
        }
    }
    return INVALID_CLIENT_ID;
}

int SV_AddClientWithNNode(nnode_t *nn) {
    assert(nn != NULL);
    for (int i = 0; i < MAX_CLIENTS; i++) {
        svClient_t *c = &ss.clients[i];
        nnode_t *cnn = c->nn;
        if (cnn != NULL) {
            continue;
        }
        memset(c, 0, sizeof(*c));
        c->nn = nnode_dup(nn);
        c->constate = CS_SPAWNED;
        c->id = i;

        svEntity_t *e = SV_SpawnEntity(c->id);
		//e->model = 1;
        return i;
    }
    return INVALID_CLIENT_ID;
}

svClient_t *SV_GetClientById(int id) {
	if (!IS_VALID_CLIENT_ID(id)) {
		return NULL;
	}
    return &ss.clients[id];
}

void SV_ConnAttempt(nnode_t *nn, szb_t *msg) {
    assert(nn != NULL);
    assert(msg != NULL);
    int i = SV_FindClientByNNode(nn);
    if (IS_VALID_CLIENT_ID(i)) {
        CON_ERR("already connected"); // FIXME: should be ignored because will happen
        return;
    }
    i = SV_AddClientWithNNode(nn);
	svClient_t *c = SV_GetClientById(i);
	c->lastPacketTime = g_Time;
    if (var_dbconn->integer != 0) {
        con_printf("sv conn ok\n");
    }
}

void SV_Connless(nnode_t *nn, szb_t *msg) {
    assert(nn != NULL);
    assert(msg != NULL);
    while (!szb_eof(msg)) {
        int cmd = szb_read8(msg);
        switch (cmd) {
            case C2S_CONNECT:
                //con_printf("conn req\n");
                SV_ConnAttempt(nn, msg);
                break;
			case C2S_DISCONNECT:
				{
					int i = SV_FindClientByNNode(nn);
					if (IS_VALID_CLIENT_ID(i)) {
						SV_SilentDrop(SV_GetClientById(i));
					}
				}
				break;

            default:
                CON_ERR("invalid cmd");
                break;
        }
    }
}

void SV_SilentDrop(svClient_t *c) {
	assert(c != NULL);
	SV_RemoveEntity(c->id);
    nnode_free(c->nn);
	c->nn = NULL;
	memset(c, 0, sizeof(*c));
	con_printf("drop\n");
}
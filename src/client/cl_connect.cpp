#include <string>
#include <cstring>
#include <cassert>
#include <cstdlib>
#include "../defs.h"

void CL_ForceDisconnect() {
	cs.server.disconCountdown = 0;
	// implicitly calls CL_SilentDisconnect() because coutndown <= 0
	CL_SendDisconnectPacket();
}

bool CL_Connect(const char *host) {
	//assert(
	if (cs.server.constate != CS_DISCONNECTED) {
        CON_ERR("client already connected");
        return false;
    }
    if (cs.online) {
        CON_ERR("client socket already allocated");
        return false;
    }
	cs.server.nn = nnode_make();
    if (!nnode_setHost(cs.server.nn, host)) {
        CON_ERR("unable to resolve host name");
        nnode_free(cs.server.nn);
        cs.server.nn = NULL;
        return false;
    }
    net_startClient();
	cs.online = true;
	cs.server.constate = CS_CONNECTING;
	if (cs.server.host != NULL) {
		delete[] cs.server.host;
	}
	cs.server.host = my_strdup(host);
    cs.server.connAttempts = var_cl_connattempts->integer;
    return true;
}

bool CL_SilentDisconnect() {
	if (!cs.online) {
        return false;
    }
	if (cs.server.nn != NULL) {
        nnode_free(cs.server.nn);
        cs.server.nn = NULL;
    }
	cs.server.constate = CS_DISCONNECTED;
    net_stopClient();
	cs.online = false;
	cs.spawned = false;
    return true;
}

void CL_SendDisconnectPacket() {
	szb_t *msg = szb_create();
    szb_write8(msg, C2S_CONNLESS);
    szb_write8(msg, C2S_DISCONNECT);
    net_send(NS_CLIENT, cs.server.nn, msg);
	szb_free(msg);
	cs.server.disconCountdown--;
	if (cs.server.disconCountdown <= 0) { 
		CL_SilentDisconnect();
	}
}

bool CL_Disconnect() {
	if (!cs.online) {
        CON_ERR("client socket not allocated");
        return false;
    }
	cs.spawned = false;
	cs.server.constate = CS_DISCONNECTING;
	cs.server.disconCountdown = 3;
    return true;
}

void CL_ConnEstablished() {
	cs.server.probableTime = -500;
	cs.server.constate = CS_SPAWNED;
    con_printf("client: connection successful\n");
}

void CL_ConnectAttempt() {
    assert(cs.server.constate == CS_CONNECTING);
	if (--cs.server.connAttempts <= 0) {
		CL_SilentDisconnect();
		con_printf("unable to connect to %s\n", cs.server.host);
		return;
        //cs.server.constate = CS_DISCONNECTED;
    }
	szb_t *msg = szb_create();
    szb_write8(msg, C2S_CONNLESS);
    szb_write8(msg, C2S_CONNECT);
    net_send(NS_CLIENT, cs.server.nn, msg);
	szb_free(msg);
	//con_printf("client: trying to connect to %s\n", cs.server.host);
}
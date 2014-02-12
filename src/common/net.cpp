#include <cassert>
#include <cstring>
#include <cerrno>
#include "../defs.h"

#ifdef _WIN32
    #include <winsock2.h>
    #pragma comment(lib,"ws2_32.lib")
    typedef int socklen_t;

static bool SocketStartup() {
    WSADATA wsa;
    WSAStartup(MAKEWORD(2, 2), &wsa); //Shoudl test eorror
	return true;
}

static void SocketCleanup() {
    WSACleanup();
}

static void SocketClose(SOCKET s) {
    closesocket(s);
}

static SOCKET SocketOpen(int port) {
    SOCKET s = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP); // FIXME: zero?
	u_long iMode=1;
	ioctlsocket(s, FIONBIO, &iMode);
	if (s != INVALID_SOCKET) {
		sockaddr_in sin;
		memset(&sin, 0, sizeof(sin));
		sin.sin_family = AF_INET;
		sin.sin_port = htons(port);
		sin.sin_addr.s_addr = htonl(INADDR_ANY);
		if (bind(s, (sockaddr *)&sin, sizeof(sin)) == -1) {
			CON_ERR("bind error");
			SocketClose(s);
			return INVALID_SOCKET;
		}
	} else {
		CON_ERR("socket error");
	}
	return s;
}

int SocketReceive(SOCKET s, char *buf, int len, sockaddr *from, int *fromlen) {
	int reclen;

	reclen = recvfrom(s,
        buf,
        len,
		0,
        from,
        fromlen);
    if (reclen == SOCKET_ERROR) {
		int error = WSAGetLastError();
        if (error != WSAEWOULDBLOCK && error != WSAECONNRESET) { // maybe should treat connreset (sendto had dest unreachable)
            CON_ERRS("recvfrom returned -1 (%d)", error);
        }
        return 0;
    }
	return reclen;
}

int SocketSend(SOCKET s, const char *buf, int len, const sockaddr* to, int tolen) {
	int sndlen;

	sndlen = sendto(s,
		buf,
		len,
		0,
		to,
		tolen);

    if (sndlen == SOCKET_ERROR) {
		//int error = WSAGetLastError();
        //if (error != WSAEWOULDBLOCK) {
        CON_ERR("sendto returned -1");
        //}
        return 0;
    }
	return sndlen;
}

int StringToAddr(const char *host, sockaddr_in *sin) {
	unsigned long addr;

	addr = inet_addr(host);
	sin->sin_addr.S_un.S_addr = addr;
	if (addr == INADDR_NONE || addr == INADDR_ANY) {
		return 0;
	}
	return 1;
}


#else
	#include <arpa/inet.h>
	#include <unistd.h>
    #define SOCKET_ERROR    -1
	#define INVALID_SOCKET  -1
    typedef int SOCKET;
// maybe define SOCKET int

static bool SocketStartup() {
	return true;
}

static void SocketCleanup() {
}

static void SocketClose(SOCKET s) {
    close(s);
}

static SOCKET SocketOpen(int port) {
	SOCKET s = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (s != INVALID_SOCKET) {
		sockaddr_in sin;
		memset(&sin, 0, sizeof(sin));
		sin.sin_family = AF_INET;
		sin.sin_port = htons(port);
		sin.sin_addr.s_addr = htonl(INADDR_ANY);
		if (bind(s, (sockaddr *)&sin, sizeof(sin)) == -1) {
			CON_ERR("bind error");
			SocketClose(s);
			return INVALID_SOCKET;
		}
	} else {
		CON_ERR("socket error");
	}
    return s;
}

int StringToAddr(const char *host, sockaddr_in *sin) {
	return inet_aton(host, &sin->sin_addr);
}

int SocketReceive(SOCKET s, char *buf, int len, sockaddr *from, socklen_t *fromlen) {
	int reclen;

	reclen = recvfrom(s,
        buf,
        len,
		MSG_DONTWAIT,
        from,
        fromlen);
    if (reclen == SOCKET_ERROR) {
        if (errno != EAGAIN && errno != EWOULDBLOCK) {
            CON_ERR("recvfrom returned -1");
        }
        return false;
    }
	return reclen;
}

int SocketSend(SOCKET s, const char *buf, int len, const sockaddr* to, int tolen) {
	int sndlen;

	sndlen = sendto(s,
		buf,
		len,
		0,
		to,
		tolen);
    if (sndlen == SOCKET_ERROR) {
        CON_ERR("sendto returned -1");
        return false;
    }
	return sndlen;
}

#endif

cvar_t *var_dbconn;

#define GAME_PORT   20333

struct nnode_t {
    sockaddr_in sin;
};

static nnode_t static_nnode;
static SOCKET sockets[2];

nnode_t *nnode_make()
{
    return new nnode_t();
}

void nnode_free(nnode_t *nn)
{
    assert(nn != NULL);
    delete nn;
}

nnode_t *nnode_dup(nnode_t *nn)
{
    nnode_t *a = nnode_make();
    *a = *nn;
    return a;
}

int nnode_cmp(nnode_t *a, nnode_t *b)
{
    return memcmp(a, b, sizeof(a->sin));
}

bool nnode_setHost(nnode_t *nn, const char *host)
{
    memset(&nn->sin, 0, sizeof(nn->sin));
    nn->sin.sin_family = AF_INET;
    nn->sin.sin_port = htons(GAME_PORT);
    if (StringToAddr(host, &nn->sin) == 0) {
        return false;
    }
    return true;
}

bool net_send(int ns, nnode_t *nn, szb_t *msg)
{
	assert(ns == NS_CLIENT || ns == NS_SERVER);
    assert(nn != NULL);

    if (sockets[ns] == INVALID_SOCKET) {
        CON_ERR("socket not started");
        return false;
    }
    int sndlen;
    sndlen = SocketSend(sockets[ns],
        (const char *)szb_getPtr(msg),
        int(szb_getSize(msg)),
        (sockaddr *)&nn->sin,
        sizeof(sockaddr_in));
    return true;
}

bool net_recv(int ns, nnode_t **nn, szb_t *msg)
{
	assert(ns == NS_CLIENT || ns == NS_SERVER);
    assert(msg != NULL);
    int reclen;
    socklen_t slen = sizeof(sockaddr_in);

    if (sockets[ns] == INVALID_SOCKET) {
        CON_ERR("socket not started");
        return false;
    }
    reclen = SocketReceive(sockets[ns],
        (char *)szb_getPtr(msg),
        int(szb_getSize(msg)),
        (sockaddr *)&static_nnode.sin,
        &slen);
    if (reclen == 0) {
        return false;
    }
    if (nn != NULL) {
        *nn = &static_nnode;
    }
    szb_rewind(msg);
    szb_resize(msg, reclen);
    return true;
}

void net_init() {
	SocketStartup();
    sockets[NS_SERVER] = INVALID_SOCKET;
    sockets[NS_CLIENT] = INVALID_SOCKET;
    //cvar_set("debugConn", "false");
	var_dbconn = cvar_register("dbconn", "0");
    CON_OK();
}

void net_cleanup()
{
    if (sockets[NS_SERVER] != INVALID_SOCKET) {
        net_stopServer();
    }
    if (sockets[NS_CLIENT] != INVALID_SOCKET) {
        net_stopClient();
    }
}

bool net_startServer()
{
    if (sockets[NS_SERVER] != INVALID_SOCKET) {
        CON_ERR("server already started");
        return false;
    }
	sockets[NS_SERVER] = SocketOpen(GAME_PORT);
	if (sockets[NS_SERVER] != INVALID_SOCKET) {
		CON_OK();
		return true;
	}
    return false;
}

bool net_stopServer()
{
    if (sockets[NS_SERVER] == INVALID_SOCKET) {
        CON_ERR("server not started");
        return false;
    }
    SocketClose(sockets[NS_SERVER]);
    sockets[NS_SERVER] = INVALID_SOCKET;
    return true;
}

bool net_startClient()
{
    if (sockets[NS_CLIENT] != INVALID_SOCKET) {
        CON_ERR("client already started");
        return false;
    }
    sockets[NS_CLIENT] = SocketOpen(0);
	if (sockets[NS_CLIENT] != INVALID_SOCKET) {
		CON_OK();
		return true;
	}
    return false;
}

bool net_stopClient()
{
    if (sockets[NS_CLIENT] == INVALID_SOCKET) {
        CON_ERR("client not started");
        return false;
    }
    SocketClose(sockets[NS_CLIENT]);
    sockets[NS_CLIENT] = INVALID_SOCKET;
    return true;
}

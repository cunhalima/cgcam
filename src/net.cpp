#include <cstdio>
#include <cstring>
#include <cassert>
#include <cstdlib>
//#include <netinet/in.h>
#ifdef WIN32
    #include <winsock2.h>
    #pragma comment(lib,"ws2_32.lib")
#else
    #include <arpa/inet.h>
    #include <sys/socket.h>
    #include <unistd.h>
#endif

//#include <netinet/in.h>
#include "defs.h"

#define SERVER_PORT 32001
#define MSG_CONNECT 1
#define MSG_DISCONNECT 2

struct client_t {
    bool used;
    struct sockaddr_in addr;
} clients[MAX_CLIENTS];

struct server_t {
    bool connected;
    struct sockaddr_in addr;
} server;

static int sv_sockfd, cl_sockfd;

void SocketInit()
{
#ifdef WIN32
    WSADATA wsa;

    WSAStartup(MAKEWORD(2,2),&wsa); //Shoudl test eorror
#endif
}

void SocketEnd()
{
#ifdef WIN32
    WSACleanup();
#endif
}

void CL_Send(MSG_t *msg)
{
    assert(msg != NULL);
    sendto(cl_sockfd, msg->data, MSG_SIZE, 0, (struct sockaddr *)&server.addr, sizeof(server.addr));
}

bool CL_Read(MSG_t *msg)
{
    int res;

    assert(msg != NULL);
    res = recvfrom(cl_sockfd, msg->data, MSG_SIZE, MSG_DONTWAIT, NULL, NULL);
    if (res == -1) {
        //if (errno == EAGAIN || errno == EWOULDBLOCK) {
        //    break;
        //}
        //break;
        return false;
    }
    if (msg->data[0] == MSG_CONNECT) {
        g_ClientId = msg->data[1];
        server.connected = true;
        printf("CL: Connect OK ID=%d\n", g_ClientId);
    } else if (msg->data[0] == MSG_DISCONNECT) {
        printf("CL: Disconnect received\n");
        server.connected = false;
    }
    return true;
}

void SV_Send(MSG_t *msg)
{
    int to = msg->to - 1;

    assert(msg != NULL);
    if (to < 0 || to >= MAX_CLIENTS) {
        return;
    }
    if (!clients[to].used) {
        return;
    }
    sendto(sv_sockfd, msg->data, MSG_SIZE, 0, (struct sockaddr *)&clients[to].addr, sizeof(clients[to].addr));
}

void SV_BroadCast(MSG_t *msg, int notto)
{
    for (int i = 0; i < MAX_CLIENTS; i++) {
        int to = i + 1;
        if (!clients[i].used || to == notto) {
            continue;
        }
        msg->to = to;
        SV_Send(msg);
    }
}

bool SV_Read(MSG_t *msg)
{
    struct sockaddr_in addr;
    socklen_t addrlen;
    int res;

    bool getnext;
    assert(msg != NULL);
    do {
        getnext = false;
        int freeslot = -1;
        addrlen = sizeof(addr);
        res = recvfrom(sv_sockfd, msg->data, MSG_SIZE, MSG_DONTWAIT, (struct sockaddr *)&addr, &addrlen);
        if (res == -1) {
            //if (errno == EAGAIN || errno == EWOULDBLOCK) {
            //    break;
            //}
            //break;
            return false;
        }
        int i;
        for (i = 0; i < MAX_CLIENTS; i++) {
            if (!clients[i].used) {
                if (freeslot == -1) {
                    freeslot = i;
                }
                continue;
            }
            if (memcmp(&addr, &clients[i].addr, sizeof(addr)) != 0) {
                continue;
            }
            //if (addr.sin_addr.sin_port != clients[i].addr.sin_port) {
            //    continue;
            //}
            break;
        }
        if (i == MAX_CLIENTS && msg->data[0] == MSG_CONNECT) {
            if (freeslot != -1) {
                i = freeslot;
                clients[i].used = true;
                memcpy(&clients[i].addr, &addr, sizeof(clients[i].addr));
                MSG_t msg2;
                msg2.data[0] = MSG_CONNECT;
                msg2.data[1] = i + 1;
                msg2.to = i + 1;
                SV_Send(&msg2);
                printf("SV: Connect Received\n");
            }
            getnext = true;
        }
        msg->from = i + 1;
        msg->to = 0;
        if (msg->data[0] == MSG_DISCONNECT) {
            getnext = true;
            clients[i].used = false;
            printf("SV: Disconnect Received\n");
        }
    } while(getnext);
    return true;
}

void SV_Start()
{
    struct sockaddr_in servaddr;
    socklen_t len;

    sv_sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(SERVER_PORT);
    bind(sv_sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr));
}

void CL_Connect(const char *ipaddr)
{
    assert(ipaddr != NULL);
    //struct sockaddr_in servaddr;
    if (server.connected) {
        return;
    }
    socklen_t len;

    bzero(&server.addr,sizeof(server.addr));
    server.addr.sin_family = AF_INET;
    server.addr.sin_addr.s_addr = inet_addr(ipaddr);
    server.addr.sin_port = htons(SERVER_PORT);

    MSG_t msg;

    msg.data[0] = MSG_CONNECT;
    printf("CL: Trying to connect... to %s\n", ipaddr);
    CL_Send(&msg);
    //bzero(&servaddr, sizeof(servaddr));
    //servaddr.sin_family = AF_INET;
    //servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    //servaddr.sin_port = htons(SERVER_PORT);
    //bind(cl_sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr));
}

void CL_Disconnect()
{
    MSG_t msg;

    if (!server.connected) {
        return;
    }
    printf("CL: Trying to disconnect...\n");
    msg.data[0] = MSG_DISCONNECT;
    CL_Send(&msg);
}

void CL_Start()
{
    cl_sockfd = socket(AF_INET, SOCK_DGRAM, 0);

    bzero(&server.addr,sizeof(server.addr));
    server.addr.sin_family = AF_INET;
    server.addr.sin_addr.s_addr = htonl(INADDR_ANY);
    server.addr.sin_port = htons(SERVER_PORT);
    server.connected = false;
}

void SV_Free()
{
    MSG_t msg;
    msg.data[0] = MSG_DISCONNECT;
    printf("SV: Disconnecting everyone...\n");
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (!clients[i].used) {
            continue;
        }
        msg.to = i + 1;
        SV_Send(&msg);
    }
    #ifdef WIN32
        closesocket(sv_sockfd);
    #else
        close(sv_sockfd);
    #endif
}

void CL_Free()
{
    if (server.connected) {
        CL_Disconnect();
    }
    #ifdef WIN32
        closesocket(cl_sockfd);
    #else
        close(cl_sockfd);
    #endif
}

#pragma once
#define _WINSOCK_DEPRECATED_NO_WARNINGS


#include <WinSock2.h>
#include <Windows.h>

#include <string.h>
#include <stdio.h>

#include "Error.h"

#pragma comment(lib, "Ws2_32.lib")

#define BACKLOG (5)

#define BUFFER_LEN (4096)

typedef struct client_s {
	int alive;
	SOCKET sock;
	struct sockaddr_in addr;
	CHAR recvBuffer[BUFFER_LEN];
} client_t;

typedef struct server_s {
	SOCKET acceptSocket;
	PCHAR* ip;
	USHORT port;
	client_t socketArray[FD_SETSIZE];
} server_t;

void cleanupServer(server_t* server);

// If ip is NULL, then all interfaces are used
error_t initServer(server_t* server, PCHAR ip, USHORT port);
error_t startServer(server_t* server);

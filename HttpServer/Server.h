#pragma once
#define _WINSOCK_DEPRECATED_NO_WARNINGS


#include <WinSock2.h>
#include <Windows.h>

#include <string.h>
#include <stdio.h>

#include "Error.h"
#include "Buffer.h"
#include "Utils.h"
#include "FileHandler.h"
#include "HttpApplication.h"

#pragma comment(lib, "Ws2_32.lib")

#define BACKLOG (5)

typedef struct client_s {
	int alive;
	SOCKET sock;
	struct sockaddr_in addr;
	buffer_t* buffer;
} client_t;

typedef struct server_s {
	SOCKET acceptSocket;
	PCHAR* ip;
	USHORT port;
	client_t clients[FD_SETSIZE];
} server_t;

// If ip is NULL, then all interfaces are used
error_t initServer(server_t* server, PCHAR ip, USHORT port, char* rootPath);
error_t startServer(server_t* server);

void cleanupServer(server_t* server);


error_t initClient(client_t* client, SOCKET sock, struct sockaddr_in addr);
void cleanupClient(client_t* client);

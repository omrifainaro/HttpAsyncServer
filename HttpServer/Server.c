#include "Server.h"

error_t initServer(server_t* server, PCHAR ip, USHORT port) {
	error_t err = ERROR_UNINIT;
	int iResult = 0;
	WSADATA wsaData = { 0 };
	
	if (!server) {
		goto fail;
	}

	ip = (ip) ? ip : "0.0.0.0";

	server->acceptSocket = INVALID_SOCKET;
	server->ip = _strdup(ip);
	server->port = port;
	memset(server->socketArray, 0, sizeof(server->socketArray));

	iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (iResult != 0) {
		printf("WSAStartup failed: %d\n", iResult);
		err = ERROR_INIT_FAILURE;
		goto fail;
	}

	server->acceptSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (server->acceptSocket == INVALID_SOCKET) {
		printf("Socket creation failed: %lu\n", GetLastError());
		err = ERROR_INIT_FAILURE;
		goto fail;
	}

	err = ERROR_OK;
	return err;

fail:
	cleanupServer(server);
	return err;
}

static error_t findFirstClient(server_t* server, client_t** outClient) {
	int i = 0;

	for (i = 0; i < FD_SETSIZE; i++) {
		if (!server->socketArray[i].alive) {
			*outClient = &server->socketArray[i];
			return ERROR_OK;
		}
	}

	*outClient = NULL;
	return ERROR_NO_FOUND;
}

static void printStringByLength(char* str, int len) {
	int i = 0;
	for (i = 0; i < len; i++) {
		printf("%c", str[i]);
	}
	printf("\n");
}

static error_t serverLoop(server_t* server) {
	error_t err = ERROR_UNINIT;
	int i = 0;
	int iResult = 0;
	fd_set readfds = { 0 };
	struct timeval tv = { 100, 0 };
	client_t* currentClient = NULL;

	while (1) {
		FD_ZERO(&readfds);
		FD_SET(server->acceptSocket, &readfds);

		for (i = 0; i < FD_SETSIZE; i++) {
			if (server->socketArray[i].alive)
				FD_SET(server->socketArray[i].sock, &readfds);
		}
		
		iResult = select(0, &readfds, NULL, NULL, &tv);
		if (iResult == SOCKET_ERROR) {
			printf("Socket error: %lu", GetLastError());
			//TODO: Think what to do here
		}
		
		// We are accepting a client
		if (FD_ISSET(server->acceptSocket, &readfds)) {
			err = findFirstClient(server, &currentClient);
			if (IS_SUCCESS(err)) {
				currentClient->sock = accept(server->acceptSocket, &currentClient->addr, NULL);
				if (currentClient->sock == INVALID_SOCKET) {
					// TODO: Think what to do here
				}
				currentClient->alive = 1;
				printf("New client connected!\n");
			}
		}

		for (i = 0; i < FD_SETSIZE; i++) {
			if (server->socketArray[i].alive) {
				if (FD_ISSET(server->socketArray[i].sock, &readfds)) {
					iResult = recv(server->socketArray[i].sock, server->socketArray[i].recvBuffer, BUFFER_LEN, 0);
					printf("Received %d bytes\n", iResult);
					printStringByLength(server->socketArray[i].recvBuffer, iResult);
				}
			}
		}
	}
}

error_t startServer(server_t* server) {
	error_t err = ERROR_UNINIT;
	int iResult = 0;
	int NonBlock = 1;
	struct sockaddr_in addr = { 0 };

	if (!server) {
		goto fail;
	}

	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = inet_addr(server->ip);
	addr.sin_port = htons(server->port);

	if (addr.sin_addr.s_addr == INADDR_NONE) {
		printf("ERROR bad ip addr %s\n", server->ip);
		err = ERROR_SOCKET;
		goto fail;
	}

	iResult = bind(server->acceptSocket, (struct sockaddr*)&addr, sizeof(addr));
	if (iResult == SOCKET_ERROR) {
		printf("ERROR Binding with %lu\n", GetLastError());
		err = ERROR_SOCKET;
		goto fail;
	}

	iResult = listen(server->acceptSocket, BACKLOG);
	if (iResult == SOCKET_ERROR) {
		printf("ERROR Listening with %lu\n", GetLastError());
		err = ERROR_SOCKET;
		goto fail;
	}
	
	//Set the socket to non blocking 
	if (ioctlsocket(server->acceptSocket, FIONBIO, &NonBlock) == SOCKET_ERROR) {
		printf("ioctlsocket() failed with error %d\n", WSAGetLastError());
		err = ERROR_SOCKET;
		goto fail;
	}

	err = serverLoop(server);
fail:
	cleanupServer(server);
	return err;
}

void cleanupServer(server_t* server) {
	if (server) {
		if (server->acceptSocket != INVALID_SOCKET)
			closesocket(server->acceptSocket);
		if (server->ip)
			free(server->ip);
	}
}
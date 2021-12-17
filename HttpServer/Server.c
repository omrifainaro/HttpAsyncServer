#include "Server.h"

error_t initServer(server_t* server, PCHAR ip, USHORT port, char* rootPath) {
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
	memset(server->clients, 0, sizeof(server->clients));

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

	initFileHandler(rootPath);

	err = ERROR_OK;
	return err;

fail:
	cleanupServer(server);
	return err;
}

static error_t findFirstClient(server_t* server, client_t** outClient) {
	int i = 0;

	for (i = 0; i < FD_SETSIZE; i++) {
		if (!server->clients[i].alive) {
			*outClient = &server->clients[i];
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

static error_t recvFromClientSocket(client_t* client) {
	error_t err = ERROR_UNINIT;
	ULONG bytes = -1;
	int iResult = 0;
	BYTE* data = NULL;

	iResult = ioctlsocket(client->sock, FIONREAD, &bytes);
	if (iResult) {
		printf("ioctlsocket failed with error: %ld\n", iResult);
		return ERROR_SOCKET;
	}

	printf("Client sent %ld bytes\n", bytes);
	data = malloc(bytes);
	iResult = recv(client->sock, data, bytes, 0);
	if (bytes != iResult) {
		err = ERROR_SOCKET;
		goto cleanup;
	}

	err = writeBuffer(client->buffer, data, iResult);

cleanup:
	if (data)
		free(data);
	return err;
}

static error_t checkFullData(client_t* client) {
	error_t err = ERROR_UNINIT;
	BYTE* ptr = NULL;
	BYTE* buffer = GET_BUF_PTR(client->buffer);
	BYTE* value = NULL;

	ptr = memnmem(buffer, client->buffer->size, HTTP_BODY_DELIM, sizeof(HTTP_BODY_DELIM) - 1);
	if (!ptr) {
		return ERROR_MORE_READ;
	}

	err = getHeaderValue(buffer, client->buffer->size, "Content-Length", &value);
	if (err == ERROR_NO_FOUND) {
		return ERROR_OK;
	}

	// If data up until \r\n\r\n
	if (client->buffer->size - ((ptr + (sizeof(HTTP_BODY_DELIM) - 1)) - buffer) >= atoi(value)) {
		return ERROR_OK;
	}
		
	return ERROR_MORE_READ;
}

static error_t handleRequest(client_t* client, http_request_t* request) {
	error_t err = ERROR_UNINIT;
	char* responseData = NULL;
	SIZE_T responseLen = 0;

	err = handleHTTPRequest(request, &responseData, &responseLen);
	if (IS_SUCCESS(err))
		send(client->sock, responseData, responseLen, 0);
	
	return err;
}

/// <summary>
/// This function gets called when there is data on the socket
/// </summary>
/// <param name="client">contains a socket and a buffer to read data to</param>
/// <returns>error code</returns>
static error_t handleClient(client_t* client) {
	http_request_t request = { 0 };

	error_t err = recvFromClientSocket(client);

	if (!IS_SUCCESS(err)) {
		return err;
	}

	err = checkFullData(client);
	if (!IS_SUCCESS(err)) {
		return err;
	}

	err = parseHttpRequest(&request, client->buffer);
	if (!IS_SUCCESS(err)) {
		return err;
	}

	err = handleRequest(client, &request);
	if (!IS_SUCCESS(err)) {
		return err;
	}

	cleanupRequest(&request);
	cleanupBuffer(client->buffer);
	initBuffer(client->buffer);

	return err;
}

static error_t serverLoop(server_t* server) {
	error_t err = ERROR_UNINIT;
	int i = 0;
	int iResult = 0;
	fd_set readfds = { 0 };
	struct timeval tv = { 100, 0 };
	SOCKET cSocket = INVALID_SOCKET;
	struct sockaddr_in addr = { 0 };
	client_t* currentClient = NULL;

	while (1) {
		FD_ZERO(&readfds);
		FD_SET(server->acceptSocket, &readfds);

		// Init readfds according to the connected clients array
		for (i = 0; i < FD_SETSIZE; i++) {
			if (server->clients[i].alive)
				FD_SET(server->clients[i].sock, &readfds);
		}
		
		iResult = select(0, &readfds, NULL, NULL, &tv);
		if (iResult == SOCKET_ERROR) {
			printf("Socket error: %lu", GetLastError());
			exit(0);
		}
		
		// We are accepting a client
		if (FD_ISSET(server->acceptSocket, &readfds)) {
			err = findFirstClient(server, &currentClient);
			if (IS_SUCCESS(err)) {
				cSocket = accept(server->acceptSocket, &addr, NULL);
				err = initClient(currentClient, cSocket, addr);
				if (!IS_SUCCESS(err)) {
					printf("Client failed to connect with error: %d\n");
					cleanupClient(currentClient);
				}
				else {
					printf("New client connected!\n");
				}
			}
			else {
				printf("No more room for new clients (max clients:%d)!\n", FD_SETSIZE);
			}
		}

		// Handle events on new client data
		for (i = 0; i < FD_SETSIZE; i++) {
			currentClient = &server->clients[i];
			if (currentClient->alive) {
				if (FD_ISSET(currentClient->sock, &readfds)) {
					err = handleClient(currentClient);
					//if (!IS_SUCCESS(err)) {
					cleanupClient(currentClient); // After every request need to close the socket
					//}
				}
				FD_CLR(currentClient->sock, &readfds);
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
		WSACleanup();
	}
}

error_t initClient(client_t* client, SOCKET sock, struct sockaddr_in addr) {
	if (sock == INVALID_SOCKET) {
		return ERROR_SOCKET;
	}
	client->sock = sock;
	client->buffer = malloc(sizeof(buffer_t));
	initBuffer(client->buffer);
	client->alive = 1;
	memcpy(&client->addr, &addr, sizeof(struct sockaddr));
	return ERROR_OK;
}

void cleanupClient(client_t* client) {
	if (client) { 
		client->alive = 0;
		cleanupBuffer(client->buffer);
		free(client->buffer);
		if (client->sock != INVALID_SOCKET)
			closesocket(client->sock);
		memset(client, 0, sizeof(client_t));
	}
}
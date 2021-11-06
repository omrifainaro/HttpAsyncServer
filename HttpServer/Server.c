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

	err = writeBuffer(&client->buffer, data, iResult);

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

	ptr = memnmem(buffer, client->buffer.size, HTTP_BODY_DELIM, sizeof(HTTP_BODY_DELIM) - 1);
	if (!ptr) {
		return ERROR_MORE_READ;
	}

	err = getHeaderValue(buffer, client->buffer.size, "Content-Length", &value);
	if (err == ERROR_NO_FOUND) {
		return ERROR_OK;
	}

	// If data up until \r\n\r\n
	if (client->buffer.size - ((ptr + (sizeof(HTTP_BODY_DELIM) - 1)) - buffer) >= atoi(value)) {
		return ERROR_OK;
	}
		
	return ERROR_MORE_READ;
}

static error_t handleRequest(client_t* client, HTTP_REQUEST_PACKET* request) {
	error_t err = ERROR_UNINIT;
	BYTE* fileContent = NULL;
	char contentLength[10];
	char* response = NULL;
	EXIT_CODE code = SUCCESS;
	SIZE_T fileSize = 0;
	HEADER h1 = { "Connection", "Keep-Alive" };
	HEADER h2 = { "Keep-Alive", "timeout=5, max=999" };
	HEADER h3 = { "Content-Length", "9" };
	HEADER* headers[] = { &h1, &h2, &h3 };
	HTTP_RESPONSE_PACKET packet = { "HTTP/1.1", "200", "OK", headers, NULL, sizeof(headers) / sizeof(HEADER*) };
	char* path = request->request_target_path;

	if (!getFileContent(path, &fileContent, &fileSize)) {
		return ERROR_NO_FOUND;
	}

	snprintf(contentLength, 10, "%d", fileSize);
	h3.value = contentLength;
	packet.body = fileContent;

	code = create_http_response_packet(&packet, &response);
	if (code == SUCCESS) {
		send(client->sock, response, strlen(response), 0);
	}
	
	return ERROR_OK;
}

/// <summary>
/// This function gets called when there is data on the socket
/// </summary>
/// <param name="client">contains a socket and a buffer to read data to</param>
/// <returns>error code</returns>
static error_t handleClient(client_t* client) {
	HTTP_REQUEST_PACKET* request = NULL;
	EXIT_CODE exitCode = SUCCESS;
	error_t err = recvFromClientSocket(client);
	char* buffer = (char*) GET_BUF_PTR(client->buffer);

	if (!IS_SUCCESS(err)) {
		return err;
	}

	err = checkFullData(client);
	if (IS_SUCCESS(err)) {
		exitCode = parse_http_request_packet(buffer, client->buffer.size, &request);
		err = handleRequest(client, request);
	}
	return err;
}

static error_t serverLoop(server_t* server) {
	error_t err = ERROR_UNINIT;
	int i = 0;
	int iResult = 0;
	fd_set readfds = { 0 };
	struct timeval tv = { 100, 0 };
	client_t* currentClient = NULL;
	client_t* clientOp = NULL;

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
			exit(0);
		}
		
		// We are accepting a client
		if (FD_ISSET(server->acceptSocket, &readfds)) {
			err = findFirstClient(server, &currentClient);
			if (IS_SUCCESS(err)) {
				currentClient->sock = accept(server->acceptSocket, &currentClient->addr, NULL);
				if (currentClient->sock == INVALID_SOCKET) {
					exit(0);
				}
				initBuffer(&currentClient->buffer);
				currentClient->alive = 1;
				printf("New client connected!\n");
			}
		}

		for (i = 0; i < FD_SETSIZE; i++) {
			clientOp = &server->socketArray[i];
			if (clientOp->alive) {
				if (FD_ISSET(clientOp->sock, &readfds)) {
					err = handleClient(clientOp);
					if (!IS_SUCCESS(err)) {
						closesocket(clientOp->sock);
						clientOp->alive = 0;
					}
				}
				FD_CLR(clientOp->sock, &readfds);
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
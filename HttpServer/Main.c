#include "Server.h"

#define LISTEN_PORT 9090
#define STATIC_FOLDER "C:/Temp/static"

int main() {
	server_t server;

	printf("Server started on port %d...\nWith work folder: %s\n", LISTEN_PORT, STATIC_FOLDER);
	initServer(&server, NULL, LISTEN_PORT, STATIC_FOLDER);
	startServer(&server);
}
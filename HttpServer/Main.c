#include "Server.h"

int main() {
	server_t server;

	initServer(&server, NULL, 8080, "C:/Temp/static");
	startServer(&server);
}
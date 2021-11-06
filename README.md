# HTTPAsyncServer

Welcome to the best async windows server alive! Here is an example main for public use:

```c
#include "Server.h"

int main() {
	server_t server;

	initServer(&server, NULL, 8080, "C:/Temp/static");
	startServer(&server);
}
```

Store all of your files in C:/Temp/static

Have fun!!!
#pragma once

#include "re.h"

#include "FileHandler.h"

#include "Structs.h"

typedef struct http_route_s {
	char* pattern;
	method_t method;
	error_t(*handler)(http_request_t* request, char** out, SIZE_T* outLen);
} http_route_t;

error_t doRoute(http_request_t* request, char** out, SIZE_T* outLen);

error_t defaultHandler(http_request_t* request, char** out, SIZE_T* outLen);
error_t sayHello(http_request_t* request, char** out, SIZE_T* outLen);
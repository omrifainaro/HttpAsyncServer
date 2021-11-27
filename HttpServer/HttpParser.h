#pragma once

#include <Windows.h>

#include "Http.h"
#include "HttpConsts.h"
#include "Buffer.h"
#include "Error.h"

#define MAX_HEADERS (32)

#define CONTENT_LENGTH "Content-Length"
#define CONTENT_TYPE "Content-Type"

typedef enum method_e {
	M_GET, 
	M_POST, 
	M_PUT, 
	M_HEAD, 
	M_OPTIONS,
	M_INVALID
} method_t;

typedef struct http_request_s {
	method_t method;
	char* path;
	header_t headers[MAX_HEADERS];
	SIZE_T contentLength;
	char* contentType;
	int curHeader;
	SIZE_T headersCount;
	BYTE* body;
} http_request_t;

error_t parseHttpRequest(http_request_t* request, buffer_t* buffer);
void cleanupRequest(http_request_t* request);
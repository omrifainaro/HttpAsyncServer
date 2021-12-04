#pragma once

#include "Consts.h"

typedef struct buffer_s {
	union {
		BYTE smallBuf[BUFF_SIZE];
		BYTE* largeBuf;
	} b;
	BYTE* end;
	BYTE* readPtr;
	SIZE_T size;
	SIZE_T capacity;
} buffer_t;

typedef enum method_e {
	M_GET,
	M_POST,
	M_PUT,
	M_HEAD,
	M_OPTIONS,
	M_INVALID
} method_t;

typedef struct header_s {
	PCHAR key;
	SIZE_T keySize;
	PCHAR value;
	SIZE_T valueSize;
} header_t;

typedef struct http_response_s {
	char* reponseLine;
	buffer_t headers;
	BYTE* body;
	SIZE_T bodySize;
} http_response_t;

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

typedef enum http_response_code_e {
	OK = 200,
	MOVED_PERMANENTLY = 301,
	BAD_REQUEST = 400,
	UNAUTHORIZED = 401,
	FORBIDDEN = 403,
	NOT_FOUND = 404,
	INTERNAL_SERVER_ERROR = 500
} http_response_code_t;


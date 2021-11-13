#pragma once

#include "HTTPConsts.h"
#include "http_parser.h"
#include "Buffer.h"
#include "Error.h"

typedef struct http_response_s {
	char* reponseLine;
	buffer_t headers;
	BYTE* body;
	SIZE_T bodySize;
} http_response_t;

void buildResponse(http_response_t* response, http_response_code_t code, char* contentType, BYTE* body, SIZE_T bodyLength);
void addHeader(http_response_t* response, char* key, char* value);
error_t responseToString(http_response_t* response, char** outData, SIZE_T* outDataLen);

// TODO: Tomer: Cleanup response
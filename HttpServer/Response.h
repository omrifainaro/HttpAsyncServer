#pragma once

#include "HTTPConsts.h"
#include "Buffer.h"
#include "Error.h"

typedef struct http_response_s {
	char* reponseLine;
	buffer_t headers;
	BYTE* body;
	SIZE_T bodySize;
} http_response_t;

// Body is copied, and will be cleaned after usage
void buildResponse(http_response_t* response, http_response_code_t code, char* contentType, BYTE* body, SIZE_T bodyLength);
void addHeader(http_response_t* response, char* key, char* value);
error_t responseToString(http_response_t* response, char** outData, SIZE_T* outDataLen);

void cleanupResponse(http_response_t* response);
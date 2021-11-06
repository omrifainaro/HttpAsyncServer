#pragma once

#include <Windows.h>

#include "Error.h"

#define HTTP_BODY_DELIM "\r\n\r\n"

typedef struct header_s {
	PCHAR key;
	SIZE_T keySize;
	PCHAR value;
	SIZE_T valueSize;
} header_t;

error_t getHeaderValue(BYTE* buffer, SIZE_T size, PCHAR header, PCHAR** outValue);
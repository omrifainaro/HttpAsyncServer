#pragma once

#include <Windows.h>

#include "Http.h"
#include "HttpConsts.h"
#include "Buffer.h"

#include "Structs.h"


error_t parseHttpRequest(http_request_t* request, buffer_t* buffer);
error_t findHeader(http_request_t* request, char* headerKey, __out int* index);
void cleanupRequest(http_request_t* request);
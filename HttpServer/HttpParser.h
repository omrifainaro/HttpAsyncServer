#pragma once

#include <Windows.h>

#include "Http.h"
#include "HttpConsts.h"
#include "Buffer.h"

#include "Structs.h"


error_t parseHttpRequest(http_request_t* request, buffer_t* buffer);
void cleanupRequest(http_request_t* request);
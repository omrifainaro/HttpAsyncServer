#pragma once

#include "Error.h"
#include "Structs.h"
#include "Routing.h"

error_t handleHTTPRequest(http_request_t* request, char** out, SIZE_T* outLen);
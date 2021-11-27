#pragma once

#include "Error.h"
#include "HttpParser.h"
#include "Response.h"
#include "FileHandler.h"

error_t handleHTTPRequest(http_request_t* request, char** out, SIZE_T* outLen);
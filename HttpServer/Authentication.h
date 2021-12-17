#pragma once
#include "Structs.h"
#include "HttpParser.h"

#define ADMIN_AUTH ("Basic YWRtaW46YWRtaW4=")

#define AUTH_HEADER ("Authorization")

error_t isAuthenticated(http_request_t* request);
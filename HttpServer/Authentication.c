#include "Authentication.h"

error_t isAuthenticated(http_request_t* request) {
	int i = 0;
	error_t err = ERROR_UNINIT;
	char* headerVal = NULL;

	err = findHeader(request, AUTH_HEADER, &i);
	if (!IS_SUCCESS(err))
		return ERROR_UNAUTHENTICATED;

	if (request->headers[i].valueSize != sizeof(ADMIN_AUTH) - 1)
		return ERROR_UNAUTHENTICATED;

	headerVal = request->headers[i].value;

	return !strncmp(headerVal, ADMIN_AUTH, sizeof(ADMIN_AUTH) - 1) ? 
		ERROR_OK : ERROR_UNAUTHENTICATED;
}
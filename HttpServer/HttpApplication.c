#include "HttpApplication.h"

static error_t handleINVALID(http_request_t* request, char** out, SIZE_T* outLen) {
	http_response_t response = { 0 };
	const char* generalError = "Yo dude, this request is shit!";
	buildResponse(&response, BAD_REQUEST, NULL, generalError, strlen(generalError), 1);
	return responseToString(&response, out, outLen);
}

error_t handleHTTPRequest(http_request_t* request, char** out, SIZE_T* outLen) {
	if (request->method == M_INVALID)
		return handleINVALID(request, out, outLen);
	return doRoute(request, out, outLen);
}
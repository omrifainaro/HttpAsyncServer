#include "HttpApplication.h"

static error_t handleGET(http_request_t* request, char** out, SIZE_T* outLen);
static error_t handlePOST(http_request_t* request, char** out, SIZE_T* outLen);
static error_t handleINVALID(http_request_t* request, char** out, SIZE_T* outLen);

struct { 
	method_t method; 
	error_t(*handler)(http_request_t* request, char** out, SIZE_T* outLen);
} methodHandlers[] = {
	{M_GET, handleGET},
	{M_POST, handlePOST},
	{M_PUT, handleINVALID},
	{M_HEAD, handleINVALID},
	{M_OPTIONS, handleINVALID},
	{M_INVALID, handleINVALID},
};


static error_t handleGET(http_request_t* request, char** out, SIZE_T* outLen) {
	error_t err = ERROR_UNINIT;
	SIZE_T fileSize = 0;
	BYTE* fileContent = NULL;
	http_response_t response = { 0 };

	char* path = request->path;
	if (!strcmp(path, "/"))
		path = "/index.html";

	if (!path)
		return ERROR_OK;

	if (!getFileContent(path, &fileContent, &fileSize)) {
		buildResponse(&response, NOT_FOUND, NULL, NULL, 0);
	}
	else {
		buildResponse(&response, OK, "text/html", fileContent, fileSize);
	}

	err = responseToString(&response, out, outLen);
	return err;
}

static error_t handlePOST(http_request_t* request, char** out, SIZE_T* outLen) {

}

static error_t handleINVALID(http_request_t* request, char** out, SIZE_T* outLen) {
	http_response_t response = { 0 };
	const char* generalError = "Yo dude, this request is shit!";
	buildResponse(&response, BAD_REQUEST, NULL, generalError, strlen(generalError));
	return responseToString(&response, out, outLen);
}

error_t handleHTTPRequest(http_request_t* request, char** out, SIZE_T* outLen) {
	return methodHandlers[request->method].handler(request, out, outLen);;
}
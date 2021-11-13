#include "Response.h"

void addHeader(http_response_t* response, char* key, char* value) {
	writeBuffer(&response->headers, key, strlen(key));
	writeBuffer(&response->headers, HEADER_DELIM, sizeof(HEADER_DELIM) - 1);
	writeBuffer(&response->headers, value, strlen(value));
	writeBuffer(&response->headers, CRLF, sizeof(CRLF) - 1);
}

static void addDefaultHeaders(http_response_t* response, SIZE_T contentLength, char* contentType) {
	char buffer[10] = { 0 };
	char* date = get_datetime();

	if (contentLength)
		_itoa(contentLength, buffer, 10);

	addHeader(response, "Connection", "Keep-Alive");
	addHeader(response, "Keep-Alive", "timeout=5, max=999");
	if (contentLength) addHeader(response, "Content-Length", buffer);
	if (contentType) addHeader(response, "Content-Type", contentType);
	addHeader(response, "Date", date);

	free(date);
}

void buildResponse(http_response_t* response, http_response_code_t code, char* contentType, BYTE* body, SIZE_T bodyLength) {
	response->reponseLine = getResponseLine(code);
	initBuffer(&response->headers);
	addDefaultHeaders(response, bodyLength, contentType);
	response->body = body;
	response->bodySize = bodyLength;
}

error_t responseToString(http_response_t* response, char** outData, SIZE_T* outDataLen) {
	buffer_t buffer;
	initBuffer(&buffer);

	if (!response) {
		return ERROR_UNINIT;
	}

	writeBuffer(&buffer, response->reponseLine, strlen(response->reponseLine));
	writeBuffer(&buffer, GET_BUF_PTR(&response->headers), response->headers.size);
	writeBuffer(&buffer, CRLF, sizeof(CRLF) - 1);
	if(response->body)
		writeBuffer(&buffer, response->body, response->bodySize);

	*outData = malloc(buffer.size);
	memcpy(*outData, GET_BUF_PTR(&buffer), buffer.size);
	*outDataLen = buffer.size;

	cleanupBuffer(&buffer);
	return ERROR_OK;
}
#include "HttpParser.h"

static method_t getMethod(BYTE* data, SIZE_T dataLen) {
	if (dataLen >= sizeof("GET") - 1 && !memcmp(data, "GET", sizeof("GET") - 1))
		return M_GET;
	if (dataLen >= sizeof("POST") - 1 && !memcmp(data, "POST", sizeof("POST") - 1))
		return M_POST;
	if (dataLen >= sizeof("OPTIONS") - 1 && !memcmp(data, "OPTIONS", sizeof("OPTIONS") - 1))
		return M_OPTIONS;
	if (dataLen >= sizeof("PUT") - 1 && !memcmp(data, "PUT", sizeof("PUT") - 1))
		return M_PUT;
	if (dataLen >= sizeof("HEAD") - 1 && !memcmp(data, "HEAD", sizeof("HEAD") - 1))
		return M_HEAD;
	return M_INVALID;
}

static int isValidPath(BYTE* data, SIZE_T dataLen) {
	SIZE_T i = 0;

	if (*data != '/')
		return ERROR_INVALID_REQUEST;

	for (i = 0; i < dataLen; i++) {
		if (!(data[i] > ' ' && data[i] <= '~'))
			return ERROR_INVALID_REQUEST;
	}

	return ERROR_OK;
}

static error_t parseRequestLine(http_request_t* request, char* data, SIZE_T dataLen) {
	char* ptr = NULL;

	ptr = memnmem(data, dataLen, " ", 1);
	if (!ptr)
		return ERROR_INVALID_REQUEST;

	request->method = getMethod(data, ptr - data);

	data = ptr + 1;
	dataLen -= ptr - data + 1;

	ptr = memnmem(data, dataLen, " ", 1);
	if (!ptr)
		return ERROR_INVALID_REQUEST;

	if (!IS_SUCCESS(isValidPath(data, ptr - data)))
		return ERROR_INVALID_REQUEST;

	request->path = myStrndup(data, ptr - data);

	return ERROR_OK;
}

static void handleSpecialHeaders(http_request_t* request, header_t* header) {
	if (header->keySize != sizeof(CONTENT_LENGTH) - 1 &&
		!strnicmp(header->key, CONTENT_LENGTH, header->keySize))
		request->contentLength = atoi(header->value);
	else if (header->keySize != sizeof(CONTENT_TYPE) - 1 &&
		!strnicmp(header->key, CONTENT_TYPE, header->keySize))
		request->contentType = myStrndup(header->value, header->valueSize);
}

static error_t parseHeaders(http_request_t* request, buffer_t* buffer) {
	error_t err = ERROR_UNINIT;
	BYTE* curHeader = NULL;
	BYTE* headerCopy = NULL;
	SIZE_T curHeaderSize = 0;
	header_t* headerPtr = NULL;
	int val = 0;
	request->curHeader = 0;

	do {
		// curHeader is a newly allocated buffer
		err = readBufferUntil(buffer, CRLF, sizeof(CRLF) - 1, &curHeader, &curHeaderSize);
		if (!IS_SUCCESS(err))
			goto cleanup;

		headerPtr = &request->headers[request->curHeader];
		val = parseHeader(curHeader, curHeaderSize, headerPtr);
		handleSpecialHeaders(request, headerPtr);

		if (!val) {
			err = ERROR_INVALID_REQUEST;
			goto cleanup;
		}

		request->curHeader++;
		if (request->curHeader >= MAX_HEADERS) {
			err = ERROR_INVALID_REQUEST;
			goto cleanup;
		}
	} while (memcmp(buffer->readPtr, CRLF, sizeof(CRLF) - 1));

cleanup:
	return err;
}

error_t parseHttpRequest(http_request_t* request, buffer_t* buffer) {
	char* requestLine = NULL;
	SIZE_T requestLineSize = 0;
	error_t err = ERROR_UNINIT;

	if (!request || !buffer)
		return err;

	err = readBufferUntil(buffer, CRLF, sizeof(CRLF) - 1, &requestLine, &requestLineSize);
	if (!IS_SUCCESS(err))
		goto cleanup;

	err = parseRequestLine(request, requestLine, requestLineSize);
	if (!IS_SUCCESS(err))
		goto cleanup;

	err = parseHeaders(request, buffer);
	if (!IS_SUCCESS(err))
		goto cleanup;

	if (request->contentLength != 0) {
		request->body = malloc(request->contentLength);
		err = readBuffer(buffer, request->body, request->contentLength); // Supposed to succeed
	}

cleanup:
	if (!IS_SUCCESS(err))
		cleanupRequest(request);
	if (requestLine)
		free(requestLine);

	return err;
}

void cleanupRequest(http_request_t* request) {
	int i = 0;

	if (!request)
		return;

	if (request->body)
		free(request->body);

	if (request->path)
		free(request->path);

	if (request->contentType)
		free(request->contentType);

	for (i = 0; i < request->headersCount; i++) {
		free(request->headers[i].key);
	}

	memset(request, 0, sizeof(*request));
}
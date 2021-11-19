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
	if (request->method == M_INVALID)
		return ERROR_INVALID_REQUEST;

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

static error_t parseHeaders(http_request_t* request, buffer_t* buffer) {
	error_t err = ERROR_UNINIT;
	BYTE* curHeader = NULL;
	BYTE* headerCopy = NULL;
	SIZE_T curHeaderSize = 0;
	int val = 0;
	request->curHeader = 0;

	do {
		err = readBufferUntil(buffer, CRLF, sizeof(CRLF) - 1, &curHeader, &curHeaderSize);
		if (!IS_SUCCESS(err))
			goto cleanup;
		headerCopy = myStrndup(curHeader, curHeaderSize);
		val = parseHeader(headerCopy, curHeaderSize, &request->headers[request->curHeader]);
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
	if (curHeader) {
		free(curHeader);
	}
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



cleanup:
	if (requestLine)
		free(requestLine);
	return err;
}
#pragma once

#include "Buffer.h"
#include "HTTPConsts.h"
#include "Structs.h"

/// <summary>
/// This function initializes the fields of the given response according
/// to the following function parameters
/// </summary>
/// <param name="response">An allocated struct to hold the response</param>
/// <param name="code">HTTP response code</param>
/// <param name="contentType">Content type -> can be NULL</param>
/// <param name="body">Body of response -> can be NULL</param>
/// <param name="bodyLength">Length of body -> can be 0</param>
/// <param name="includeDefaultHeaders"> Whether to add the default headers </param>
void buildResponse(http_response_t* response, http_response_code_t code, char* contentType, BYTE* body, SIZE_T bodyLength, int includeDefaultHeaders);

/// <summary>
/// Adds a custom header to an already built and initialized response
/// </summary>
/// <param name="response">The response to add a header to</param>
/// <param name="key">Key of the header</param>
/// <param name="value">Value of the header</param>
void addHeader(http_response_t* response, char* key, char* value);


error_t responseToString(http_response_t* response, char** outData, SIZE_T* outDataLen);

void cleanupResponse(http_response_t* response);
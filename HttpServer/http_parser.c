#include "http_parser.h"


EXIT_CODE parse_http_request_packet(char* request, int request_size, HTTP_REQUEST_PACKET** http_packet)
{
	char* token = NULL;
	char* body = NULL;
	int body_size = 0;

	(*http_packet) = (HTTP_REQUEST_PACKET*)calloc(1, sizeof(HTTP_REQUEST_PACKET));
	if (!(*http_packet)) return FAILED;

	body = strstr(request, HTTP_BODY_DELIM);
	if (!body)
	{
		printf("No \\r\\n\\r\\n in end of packet!\n");
		return BAD_REQUEST; // INVALID_REQUEST
	}

	token = strtok(request, HTTP_PARSE_DELIM); // first line of HTTP.
	if (parse_request_line(token, *http_packet) == FAILED) // parse request line.
	{
		printf("Failed to parse request line\n");
		return BAD_REQUEST;
	}

	token = strtok(NULL, HTTP_PARSE_DELIM);
	while (token < body && token) // parse headers
	{
		(*http_packet)->headers_count++;

		if (parse_header_line(token, *http_packet, (*http_packet)->headers_count)) // headers value can be more than one value.
		{
			printf("Failed to parse header line!\n");
			return BAD_REQUEST;
		}
		if (strncmp((*http_packet)->headers[(*http_packet)->headers_count - 1]->name, BODY_LEN_HEADER, strlen(BODY_LEN_HEADER)) == 0)
			body_size = atoi((*http_packet)->headers[(*http_packet)->headers_count - 1]->value) + 1; // Content-Length

		token = strtok(NULL, HTTP_PARSE_DELIM);
	}

	if (token < request + request_size && token) // if there is a body
	{
		if (copy_to_heap(&(*http_packet)->body, token, body_size) != SUCCESS)
		{
			printf("Invalid body headers!\n");
			return BAD_REQUEST;
		}
	}

	return SUCCESS;
}

EXIT_CODE parse_request_line(char* request_line, HTTP_REQUEST_PACKET* http_packet)
{
	int method_len = 0;
	char* ptr = NULL;
	char* prev_ptr = NULL;

	ptr = strchr(request_line, REQUEST_LINE_DELIM);
	if (!ptr)
		return BAD_REQUEST;

	method_len = ptr - request_line;
	if (method_len > MAX_METHOD_LEN)
		return BAD_REQUEST;

	// copy method type
	if (copy_to_heap(&http_packet->method, request_line, method_len + 1) != SUCCESS)
	{
		printf("Must give valid method type!\n");
		return BAD_REQUEST;
	}

	ptr++;
	prev_ptr = ptr;
	ptr = strchr(ptr, REQUEST_LINE_DELIM);
	if (!ptr)
		return BAD_REQUEST;

	if (ptr - prev_ptr > MAX_PATH)
		return BAD_REQUEST;
	// copy the request target - path
	if (copy_to_heap(&http_packet->request_target_path, prev_ptr, ptr - prev_ptr + 1) != SUCCESS)
	{
		printf("Must give valid path!\n");
		return BAD_REQUEST;
	}

	ptr++;
	if (request_line + strnlen(request_line, MAX_HEADER_LEN) - ptr > MAX_PROTOCOL_VERSION_LEN)
		return FAILED;
	// copy the protocol version
	if (copy_to_heap(&http_packet->protocol_version, ptr, request_line + strnlen(request_line, MAX_HEADER_LEN) - ptr + 1) != SUCCESS)
	{
		printf("Must give valid protocol version!\n");
		return BAD_REQUEST;
	}

	return SUCCESS;
}


EXIT_CODE parse_header_line(char* header_line, HTTP_REQUEST_PACKET* http_packet, int headers_count)
{
	char* ptr = NULL;
	int name_size = 0, value_size = 0;

	http_packet->headers = (HEADER**)realloc(http_packet->headers, sizeof(HEADER*) * headers_count);
	if (!http_packet->headers) return FAILED;

	ptr = strstr(header_line, HEADER_DELIM);
	if (!ptr)
		return BAD_REQUEST;

	http_packet->headers[headers_count - 1] = (HEADER*)malloc(sizeof(HEADER));
	if (!http_packet->headers[headers_count - 1]) return FAILED;

	name_size = ptr - header_line + 1;

	// copy header name
	if (copy_to_heap(&http_packet->headers[headers_count - 1]->name, header_line, name_size) != SUCCESS) // with null byte.
	{
		printf("Invalid header name!\n");
		return BAD_REQUEST;
	}
	// copy header value
	if (copy_to_heap(&http_packet->headers[headers_count - 1]->value, ptr + 2, strnlen(header_line, MAX_HEADER_LEN) - name_size) != SUCCESS) // with null byte. + 2 to not include ": "
	{
		printf("Invalid header value!\n");
		return BAD_REQUEST;
	}
	return SUCCESS;
}


EXIT_CODE create_http_response_packet(HTTP_RESPONSE_PACKET* http_packet, char** response)
{
	int packet_size = 0, i = 0, body_size = 0;

	if (get_response_packet_size(http_packet, &packet_size))
	{
		printf("Invalid http_packet to create packet!\n");
		return FAILED;
	}

	*response = (char*)calloc(packet_size, sizeof(char));
	if (!(*response)) return FAILED;
	if (http_packet->protocol_version)
	{
		strncat(*response, http_packet->protocol_version, MAX_PROTOCOL_VERSION_LEN);
		strncat(*response, SPACE, sizeof(char));
	}

	if (http_packet->status_code_int)
	{
		strncat(*response, http_packet->status_code_int, MAX_STATUS_CODE_INT_LEN);
		strncat(*response, SPACE, sizeof(char));
	}

	if (http_packet->status_code_str)
		strncat(*response, http_packet->status_code_str, MAX_STATUS_CODE_STR_LEN);


	strncat(*response, HTTP_PARSE_DELIM, strlen(HTTP_PARSE_DELIM)); // \r\n

	for (i = 0; i < http_packet->headers_count; i++)
	{
		if (http_packet->headers[i]->name)
		{
			strncat(*response, http_packet->headers[i]->name, MAX_HEADER_LEN);
			strncat(*response, HEADER_DELIM, strlen(HEADER_DELIM)); // ": "
		}
		if (http_packet->headers[i]->value)
			strncat(*response, http_packet->headers[i]->value, MAX_HEADER_LEN);

		if (strncmp(http_packet->headers[i]->name, BODY_LEN_HEADER, strlen(BODY_LEN_HEADER)) == 0)
			body_size = atoi(http_packet->headers[i]->value) + 1;

		strncat(*response, HTTP_PARSE_DELIM, strlen(HTTP_PARSE_DELIM)); // \r\n
	}
	strncat(*response, HTTP_PARSE_DELIM, strlen(HTTP_PARSE_DELIM)); // for completing the \r\n to \r\n\r\n

	if (http_packet->body)
	{
		if (body_size <= 0)
		{
			printf("Invalid body headers!\n");
			return BAD_REQUEST;
		}
		strncat(*response, http_packet->body, body_size);
	}
	return SUCCESS;
}


EXIT_CODE get_response_packet_size(HTTP_RESPONSE_PACKET* packet, int* size)
{
	int len = 0, i = 0;

	if (!packet)
		return FAILED;
	if (packet->protocol_version) // TODO: later on to check validity we can return FAILED if there is no method / path / protocol version.
		len += strnlen(packet->protocol_version, MAX_METHOD_LEN) + 1; // + 1 for space
	if (packet->status_code_int)
		len += strnlen(packet->status_code_int, MAX_PATH) + 1; // + 1 for space
	if (packet->status_code_str)
		len += strnlen(packet->status_code_str, MAX_PROTOCOL_VERSION_LEN);

	len += strlen(HTTP_PARSE_DELIM); // for \r\n

	for (i = 0; i < packet->headers_count; i++)
	{
		if (packet->headers[i]->name)
			len += strnlen(packet->headers[i]->name, MAX_HEADER_LEN);
		
		len += strlen(HEADER_DELIM); // for ": "

		if (packet->headers[i]->value)
			len += strnlen(packet->headers[i]->value, MAX_HEADER_LEN);
		len += strlen(HTTP_PARSE_DELIM); // for \r\n
	}
	len += strlen(HTTP_PARSE_DELIM); // for completing the \r\n to \r\n\r\n

	if (packet->body)
		len += strnlen(packet->body, MAX_PACKET_SIZE);

	len++;// with null byte
	*size = len;
	return SUCCESS;
}


EXIT_CODE free_request_packet(HTTP_REQUEST_PACKET* http_packet)
{
	if (!http_packet)
		return FAILED;
	if (http_packet->method)
		free(http_packet->method);
	if (http_packet->request_target_path)
		free(http_packet->request_target_path);
	if (http_packet->protocol_version)
		free(http_packet->protocol_version);
	
	for (int i = 0; i < http_packet->headers_count; i++)
	{
		free(http_packet->headers[i]->name);
		free(http_packet->headers[i]->value);
		free(http_packet->headers[i]);
	}
	if (http_packet->headers)
		free(http_packet->headers);
	if (http_packet->body)
		free(http_packet->body);

	free(http_packet);
	return SUCCESS;
}

EXIT_CODE free_response_packet(HTTP_RESPONSE_PACKET* http_packet)
{
	if (!http_packet)
		return FAILED;
	if (http_packet->protocol_version)
		free(http_packet->protocol_version);
	if (http_packet->status_code_str)
		free(http_packet->status_code_str);

	for (int i = 0; i < http_packet->headers_count; i++)
	{
		free(http_packet->headers[i]->name);
		free(http_packet->headers[i]->value);
		free(http_packet->headers[i]);
	}
	if (http_packet->headers)
		free(http_packet->headers);
	if (http_packet->body)
		free(http_packet->body);

	free(http_packet);
	return SUCCESS;
}
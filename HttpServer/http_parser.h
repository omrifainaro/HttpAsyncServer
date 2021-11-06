#pragma once
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#include <WinSock2.h>
#include <Windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "Error.h"
#include "Utils.h"

#define HTTP_PARSE_DELIM "\r\n"
#define HTTP_BODY_DELIM "\r\n\r\n"
#define HEADER_DELIM ": "
#define SPACE " "
#define REQUEST_LINE_DELIM ' '

#define BODY_LEN_HEADER "Content-Length"

#define MAX_PACKET_SIZE (65535)
#define MAX_HEADER_LEN (50)
#define MAX_METHOD_LEN (10)
#define MAX_PROTOCOL_VERSION_LEN (15)
#define MAX_STATUS_CODE_INT_LEN (3)
#define MAX_STATUS_CODE_STR_LEN (40)

typedef struct _HEADER
{
	char* name;
	char* value;
} HEADER;


typedef struct _HTTP_REQUEST_PACKET
{
	char* method;
	char* request_target_path;
	char* protocol_version;
	HEADER** headers;
	char* body;
	int headers_count;
} HTTP_REQUEST_PACKET;

typedef struct _HTTP_RESPONSE_PACKET
{
	char* protocol_version;
	char* status_code_int;
	char* status_code_str;
	HEADER** headers;
	char* body;
	int headers_count;
} HTTP_RESPONSE_PACKET;

// Requests handling
EXIT_CODE parse_http_request_packet(char* request, int request_size, HTTP_REQUEST_PACKET** http_packet);
EXIT_CODE parse_request_line(char* request_line, HTTP_REQUEST_PACKET* http_packet);
EXIT_CODE parse_header_line(char* header_line, HTTP_REQUEST_PACKET* http_packet, int headers_count);
EXIT_CODE free_request_packet(HTTP_REQUEST_PACKET* http_packet);

// Response handling
EXIT_CODE create_http_response_packet(HTTP_RESPONSE_PACKET* http_packet, char** response);
EXIT_CODE get_response_packet_size(HTTP_RESPONSE_PACKET* packet, int* size);
EXIT_CODE free_response_packet(HTTP_RESPONSE_PACKET* http_packet);
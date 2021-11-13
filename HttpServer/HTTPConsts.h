#pragma once

#define _CRT_SECURE_NO_WARNINGS
#pragma warning(disable : 4996)

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define GET_ERROR(err) #err

#define DATETIME_FORMAT "%a, %d %b %Y %T GMT"
#define DATETIME_LEN 30

#define CRLF "\r\n"
#define HEADER_DELIM ": "

typedef enum http_response_code_e {
	OK = 200,
	MOVED_PERMANENTLY = 301,
	BAD_REQUEST = 400,
	UNAUTHORIZED = 401,
	FORBIDDEN = 403,
	NOT_FOUND = 404,
	INTERNAL_SERVER_ERROR = 500
} http_response_code_t;

inline char* replace_(const char* s) {
	char* s2 = _strdup(s);
	char* current_pos = strchr(s2, '_');
	while (current_pos) {
		*current_pos = ' ';
		current_pos = strchr(current_pos, '_');
	}
	return s2;
}

inline char* getResponseLine(http_response_code_t code) {
	char buffer[128] = { 0 };
	char* s = replace_(GET_ERROR(code));
	snprintf(buffer, sizeof(buffer), "HTTP/1.1 %d %s\r\n", code, s);
	free(s);
	return _strdup(buffer);
}

inline char* get_datetime() {
	time_t timer = time(NULL);
	struct tm* timeinfo = gmtime(&timer);
	char* curr_gmt_time = (char*)calloc(DATETIME_LEN, sizeof(char));
	strftime(curr_gmt_time, DATETIME_LEN, DATETIME_FORMAT, timeinfo);
	return curr_gmt_time;
}



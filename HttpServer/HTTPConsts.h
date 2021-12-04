#pragma once

#define _CRT_SECURE_NO_WARNINGS
#pragma warning(disable : 4996)

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "Structs.h"

inline char* errorCodeToString(http_response_code_t code) {
	switch (code) {
	case OK:
		return "OK";
	case MOVED_PERMANENTLY:
		return "MOVED PERMANENTLY";
	case BAD_REQUEST:
		return "BAD REQUEST";
	case UNAUTHORIZED:
		return "UNAUTHORIZED";
	case FORBIDDEN:
		return "FORBIDDEN";
	case NOT_FOUND:
		return "NOT FOUND";
	case INTERNAL_SERVER_ERROR:
		return "INTERNAL SERVER ERROR";
	}

	return "UNKNOWN";
}

inline char* getResponseLine(http_response_code_t code) {
	char buffer[128] = { 0 };
	snprintf(buffer, sizeof(buffer), "HTTP/1.1 %d %s\r\n", code, errorCodeToString(code));
	return _strdup(buffer);
}

inline char* get_datetime() {
	time_t timer = time(NULL);
	struct tm* timeinfo = gmtime(&timer);
	char* curr_gmt_time = (char*)calloc(DATETIME_LEN, sizeof(char));
	strftime(curr_gmt_time, DATETIME_LEN, DATETIME_FORMAT, timeinfo);
	return curr_gmt_time;
}



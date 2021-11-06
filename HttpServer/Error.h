#pragma once

typedef enum error_e {
	ERROR_OK,
	ERROR_UNINIT,
	ERROR_INIT_FAILURE,
	ERROR_MEMORY,
	ERROR_SOCKET,
	ERROR_NO_FOUND,
	ERROR_BUFFER,
	ERROR_MORE_READ
} error_t;

// For ronf's library
typedef enum _EXIT_CODE
{
	SUCCESS,
	FAILED,
	BAD_REQUEST
} EXIT_CODE;

#define IS_SUCCESS(err) (err == ERROR_OK) 
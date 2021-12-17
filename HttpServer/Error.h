#pragma once

typedef enum error_e {
	ERROR_OK,
	ERROR_UNINIT,
	ERROR_INIT_FAILURE,
	ERROR_MEMORY,
	ERROR_SOCKET,
	ERROR_NO_FOUND,
	ERROR_BUFFER,
	ERROR_MORE_READ,
	ERROR_NOT_ENOUGH_DATA,
	ERROR_INVALID_REQUEST,
	ERROR_UNAUTHENTICATED
} error_t;

#define IS_SUCCESS(err) (err == ERROR_OK) 
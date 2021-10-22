#pragma once

typedef enum error_e {
	ERROR_OK,
	ERROR_UNINIT,
	ERROR_INIT_FAILURE,
	ERROR_MEMORY,
	ERROR_SOCKET,
	ERROR_NO_FOUND
} error_t;

#define IS_SUCCESS(err) (err == ERROR_OK) 
#pragma once
#define _CRT_SECURE_NO_WARNINGS

#include <Windows.h>
#include "Error.h"

BYTE* memnmem(BYTE* buffer, SIZE_T bufLen, BYTE* search, SIZE_T searchLen);
char* myStrndup(BYTE* data, SIZE_T dataLen);
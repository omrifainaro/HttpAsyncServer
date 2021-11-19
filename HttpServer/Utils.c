#include "Utils.h"

BYTE* memnmem(BYTE* buffer, SIZE_T bufLen, BYTE* search, SIZE_T searchLen) {
	BYTE* ptr = buffer;

	while (ptr + searchLen <= buffer + bufLen)
	{
		if (!memcmp(ptr, search, searchLen)) {
			return ptr;
		}
		ptr++;
	}

	return NULL;
}

char* myStrndup(BYTE* data, SIZE_T dataLen) {
	char* ptr = malloc(dataLen + 1);
	memcpy(ptr, data, dataLen);
	ptr[dataLen] = 0;
	return ptr;
}
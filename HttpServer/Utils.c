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

EXIT_CODE copy_to_heap(char** dest, char* src, int dest_size)
{
	if (dest_size <= 0) return KAK_REQUEST;
	*dest = (char*)malloc(dest_size);
	if (!*dest) return FAILED;
	strncpy(*dest, src, dest_size);
	(*dest)[dest_size - 1] = '\0';
	return SUCCESS;
}
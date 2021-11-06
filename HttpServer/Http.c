#include "Http.h"

static int parseHeader(BYTE* buffer, SIZE_T buffSize, header_t* out) {
	BYTE* ptr = buffer;
	int found = 0;

	out->key = buffer;
	while (ptr + 1 < buffer + buffSize) {
		if (*ptr == ':') {
			found = 1;
			break;
		}
		ptr++;
	}
	if (!found)
		return 0;
	found = 0;
	out->keySize = ptr - buffer;

	if (ptr >= buffer + buffSize)
		return 0;

	out->value = ptr + 1;
	while (ptr + 1 < buffer + buffSize) {
		if (*ptr == '\r') {
			found = 1;
			break;
		}
		ptr++;
	}
	if (!found)
		return 0;
	out->valueSize = ptr - buffer + out->keySize + 1;
	return 1;
}

static BYTE* iterateHeaders(BYTE* base, BYTE** iterator, SIZE_T size, header_t* out) {
	BYTE* cur = NULL;
	int found = 0;

	while (*iterator + 1 < base + size && (**iterator == '\r' || **iterator == '\n'))
		(*iterator)++;

	cur = *iterator;

	while (*iterator + 2 < base + size) {
		if ((*iterator)[0] == '\r' && (*iterator)[1] == '\n') {
			found = 1;
			break;
		}

		(*iterator)++;
	}

	if (found) {
		parseHeader(cur, *iterator - cur, out);
	}

	return (found) ? cur : NULL;
}


error_t getHeaderValue(BYTE* buffer, SIZE_T size, PCHAR header, PCHAR** outValue) {
	BYTE* iterator = buffer;
	BYTE* curHeader = NULL;
	header_t parsedHeader = {0};
	int headerLen = strlen(header);

	while (curHeader = iterateHeaders(buffer, &iterator, size, &parsedHeader)) {
		if (parsedHeader.keySize < headerLen)
			continue;
		if (!memcmp(parsedHeader.key, header, headerLen)) {
			*outValue = parsedHeader.value;
			return ERROR_OK;
		}
	}

	return ERROR_NO_FOUND;
}